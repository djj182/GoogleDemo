/****************************************************************************
 Copyright (c) 2013 cocos2d-x.org
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "Readyer.h"
#include "cocos2d.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <cstdio>
#include <cerrno>

NS_CC_EXT_BEGIN

#define LOW_SPEED_LIMIT     1L
#define LOW_SPEED_TIME      5L
#define MAX_REDIRS          2
#define DEFAULT_TIMEOUT     5
#define HTTP_CODE_SUPPORT_RESUME    206
#define MAX_WAIT_MSECS 30*1000 /* Wait max. 30 seconds */

#define TEMP_EXT            ".temp"

size_t fileWriteFunc(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    FILE *fp = (FILE*)userdata;
    size_t written = fwrite(ptr, size, nmemb, fp);
    return written;
}

size_t bufferWriteFunc(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    Readyer::StreamData *streamBuffer = (Readyer::StreamData *)userdata;
    size_t written = size * nmemb;
    // Avoid pointer overflow
    if (streamBuffer->offset + written <= static_cast<size_t>(streamBuffer->total))
    {
        memcpy(streamBuffer->buffer + streamBuffer->offset, ptr, written);
        streamBuffer->offset += written;
        return written;
    }
    else return 0;
}

// This is only for batch!!!! process, will notify file succeed event in progress function
int batchReadyProgressFunc(Readyer::ProgressData *ptr, double totalToReady, double nowReadyed, double totalToUpLoad, double nowUpLoaded)
{
    if (ptr->totalToReady == 0)
    {
        ptr->totalToReady = totalToReady;
    }
    
    if (ptr->readyed != nowReadyed)
    {
        ptr->readyed = nowReadyed;
        
        Readyer::ProgressData data = *ptr;
        
        if (nowReadyed == totalToReady)
        {
            Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
                if (!data.readyer.expired())
                {
                    std::shared_ptr<Readyer> Readyer = data.readyer.lock();
                
                    auto progressCB = Readyer->getProgressCallback();
                    if (progressCB != nullptr)
                    {
                        progressCB(totalToReady, nowReadyed, data.url, data.customId);
                    }
                    auto successCB = Readyer->getSuccessCallback();
                    if (successCB != nullptr)
                    {
                        successCB(data.url, data.path + data.name, data.customId);
                    }
                }
            });
        }
        else
        {
            Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
                if (!data.readyer.expired())
                {
                    std::shared_ptr<Readyer> Readyer = data.readyer.lock();
                
                    auto callback = Readyer->getProgressCallback();
                    if (callback != nullptr)
                    {
                        callback(totalToReady, nowReadyed, data.url, data.customId);
                    }
                }
            });
        }
    }
    
    return 0;
}

// Compare to batchReadyProgressFunc, this only handles progress information notification
int readyProgressFunc(Readyer::ProgressData *ptr, double totalToReady, double nowReadyed, double totalToUpLoad, double nowUpLoaded)
{
    if (ptr->totalToReady == 0)
    {
        ptr->totalToReady = totalToReady;
    }
    
    if (ptr->readyed != nowReadyed)
    {
        ptr->readyed = nowReadyed;
        Readyer::ProgressData data = *ptr;
        
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
            if (!data.readyer.expired())
            {
                std::shared_ptr<Readyer> Readyer = data.readyer.lock();
                
                auto callback = Readyer->getProgressCallback();
                if (callback != nullptr)
                {
                    callback(totalToReady, nowReadyed, data.url, data.customId);
                }
            }
        });
    }
    
    return 0;
}

Readyer::Readyer()
: _connectionTimeout(DEFAULT_TIMEOUT)
, _onError(nullptr)
, _onProgress(nullptr)
, _onSuccess(nullptr)
, _supportResuming(false)
{
    _fileUtils = FileUtils::getInstance();
}

Readyer::~Readyer()
{
}

int Readyer::getConnectionTimeout()
{
    return _connectionTimeout;
}

void Readyer::setConnectionTimeout(int timeout)
{
    if (timeout >= 0)
        _connectionTimeout = timeout;
}

void Readyer::notifyError(ErrorCode code, const std::string &msg/* ="" */, const std::string &customId/* ="" */, int curle_code/* = CURLE_OK*/, int curlm_code/* = CURLM_OK*/)
{
    std::weak_ptr<Readyer> ptr = shared_from_this();
    Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
        if (!ptr.expired())
        {
            std::shared_ptr<Readyer> Readyer = ptr.lock();
            if (Readyer->_onError != nullptr)
            {
                Error err;
                err.code = code;
                err.curle_code = curle_code;
                err.curlm_code = curlm_code;
                err.message = msg;
                err.customId = customId;
                Readyer->_onError(err);
            }
        }
    });
}

void Readyer::notifyError(const std::string &msg, int curlm_code, const std::string &customId/* = ""*/)
{
    notifyError(ErrorCode::CURL_MULTI_ERROR, msg, customId, CURLE_OK, curlm_code);
}

void Readyer::notifyError(const std::string &msg, const std::string &customId, int curle_code)
{
    notifyError(ErrorCode::CURL_EASY_ERROR, msg, customId, curle_code);
}

std::string Readyer::getFileNameFromUrl(const std::string &srcUrl)
{
    // Find file name and file extension
    std::string filename;
    unsigned long found = srcUrl.find_last_of("/\\");
    if (found != std::string::npos)
        filename = srcUrl.substr(found+1);
    return filename;
}

void Readyer::clearBatchReadyData()
{
    while (_progDatas.size() != 0) {
        delete _progDatas.back();
        _progDatas.pop_back();
    }
    
    while (_files.size() != 0) {
        delete _files.back();
        _files.pop_back();
    }
}

void Readyer::prepareReady(const std::string &srcUrl, const std::string &storagePath, const std::string &customId, bool resumeReady, FileDescriptor *fDesc, ProgressData *pData)
{
    std::shared_ptr<Readyer> readyer = shared_from_this();
    pData->customId = customId;
    pData->url = srcUrl;
    pData->readyer = readyer;
    pData->readyed = 0;
    pData->totalToReady = 0;
    
    fDesc->fp = nullptr;
    fDesc->curl = nullptr;
    
    Error err;
    err.customId = customId;
    
    // Asserts
    // Find file name and file extension
    unsigned long found = storagePath.find_last_of("/\\");
    if (found != std::string::npos)
    {
        pData->name = storagePath.substr(found+1);
        pData->path = storagePath.substr(0, found+1);
    }
    else
    {
        err.code = ErrorCode::INVALID_URL;
        err.message = "Invalid url or filename not exist error: " + srcUrl;
        if (this->_onError) this->_onError(err);
        return;
    }
    
    // Create a file to save file.
    const std::string outFileName = storagePath + TEMP_EXT;
    if (_supportResuming && resumeReady && _fileUtils->isFileExist(outFileName))
    {
        fDesc->fp = fopen(FileUtils::getInstance()->getSuitableFOpen(outFileName).c_str(), "ab");
    }
    else
    {
        fDesc->fp = fopen(FileUtils::getInstance()->getSuitableFOpen(outFileName).c_str(), "wb");
    }
    if (!fDesc->fp)
    {
        err.code = ErrorCode::CREATE_FILE;
        err.message = StringUtils::format("Can not create file %s: errno %d", outFileName.c_str(), errno);
        if (this->_onError) this->_onError(err);
    }
}

Readyer::HeaderInfo Readyer::prepareHeader(const std::string &srcUrl, void* header/* = nullptr */)
{
    bool headerGiven = true;
    HeaderInfo info;
    info.valid = false;
    
    if (!header)
    {
        headerGiven = false;
        header = curl_easy_init();
    }
    
    curl_easy_setopt(header, CURLOPT_URL, srcUrl.c_str());
    curl_easy_setopt(header, CURLOPT_HEADER, 1);
    curl_easy_setopt(header, CURLOPT_NOBODY, 1);
    curl_easy_setopt(header, CURLOPT_NOSIGNAL, 1);
    if (curl_easy_perform(header) == CURLE_OK)
    {
        char *url;
        char *contentType;
        curl_easy_getinfo(header, CURLINFO_EFFECTIVE_URL, &url);
        curl_easy_getinfo(header, CURLINFO_CONTENT_TYPE, &contentType);
        curl_easy_getinfo(header, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &info.contentSize);
        curl_easy_getinfo(header, CURLINFO_RESPONSE_CODE, &info.responseCode);
        
        if (contentType == nullptr || info.contentSize == -1 || info.responseCode >= 400)
        {
            info.valid = false;
        }
        else
        {
            info.url = url;
            info.contentType = contentType;
            info.valid = true;
        }
    }
    
    if (info.valid && _onHeader)
    {
        _onHeader(srcUrl, info);
    }
    else if (!info.valid)
    {
        info.contentSize = -1;
        std::string msg = StringUtils::format("Can not get content size of file (%s) : Request header failed", srcUrl.c_str());
        this->notifyError(ErrorCode::PREPARE_HEADER_ERROR, msg);
    }
    
    if (!headerGiven) {
        curl_easy_cleanup(header);
    }
    
    return info;
}

long Readyer::getContentSize(const std::string &srcUrl)
{
    HeaderInfo info = prepareHeader(srcUrl);
    return info.contentSize;
}

Readyer::HeaderInfo Readyer::getHeader(const std::string &srcUrl)
{
    return prepareHeader(srcUrl);
}

void Readyer::getHeaderAsync(const std::string &srcUrl, const HeaderCallback &callback)
{
    setHeaderCallback(callback);
    auto t = std::thread(&Readyer::prepareHeader, this, srcUrl, nullptr);
    t.detach();
}

void Readyer::readyToBufferAsync(const std::string &srcUrl, unsigned char *buffer, const long &size, const std::string &customId/* = ""*/)
{
    if (buffer != nullptr)
    {
        std::shared_ptr<Readyer> Readyer = shared_from_this();
        ProgressData pData;
        pData.customId = customId;
        pData.url = srcUrl;
        pData.readyer = Readyer;
        pData.readyed = 0;
        pData.totalToReady = 0;
        
        StreamData streamBuffer;
        streamBuffer.buffer = buffer;
        streamBuffer.total = size;
        streamBuffer.offset = 0;
        
        auto t = std::thread(&Readyer::readyToBuffer, this, srcUrl, customId, streamBuffer, pData);
        t.detach();
    }
}

void Readyer::readyToBufferSync(const std::string &srcUrl, unsigned char *buffer, const long &size, const std::string &customId/* = ""*/)
{
    if (buffer != nullptr)
    {
        std::shared_ptr<Readyer> Readyer = shared_from_this();
        ProgressData pData;
        pData.customId = customId;
        pData.url = srcUrl;
        pData.readyer = Readyer;
        pData.readyed = 0;
        pData.totalToReady = 0;
        
        StreamData streamBuffer;
        streamBuffer.buffer = buffer;
        streamBuffer.total = size;
        streamBuffer.offset = 0;
        
        readyToBuffer(srcUrl, customId, streamBuffer, pData);
    }
}

void Readyer::readyToBuffer(const std::string &srcUrl, const std::string &customId, const StreamData &buffer, const ProgressData &data)
{
    std::weak_ptr<Readyer> ptr = shared_from_this();
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        this->notifyError(ErrorCode::CURL_EASY_ERROR, "Can not init curl with curl_easy_init", customId);
        return;
    }
    
    // Ready pacakge
    curl_easy_setopt(curl, CURLOPT_URL, srcUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, bufferWriteFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, readyProgressFunc);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &data);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
    if (_connectionTimeout) curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, _connectionTimeout);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::string msg = StringUtils::format("Unable to ready file to buffer: [curl error]%s", curl_easy_strerror(res));
        this->notifyError(msg, customId, res);
    }
    else
    {
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
            if (!ptr.expired())
            {
                std::shared_ptr<Readyer> Readyer = ptr.lock();
                
                auto successCB = Readyer->getSuccessCallback();
                if (successCB != nullptr)
                {
                    successCB(data.url, "", data.customId);
                }
            }
        });
    }
    
    curl_easy_cleanup(curl);
}

void Readyer::readyAsync(const std::string &srcUrl, const std::string &storagePath, const std::string &customId/* = ""*/)
{
    FileDescriptor fDesc;
    ProgressData pData;
    prepareReady(srcUrl, storagePath, customId, false, &fDesc, &pData);
    if (fDesc.fp != NULL)
    {
        auto t = std::thread(&Readyer::ready, this, srcUrl, customId, fDesc, pData);
        t.detach();
    }
}

void Readyer::readySync(const std::string &srcUrl, const std::string &storagePath, const std::string &customId/* = ""*/)
{
    FileDescriptor fDesc;
    ProgressData pData;
    prepareReady(srcUrl, storagePath, customId, false, &fDesc, &pData);
    if (fDesc.fp != NULL)
    {
        ready(srcUrl, customId, fDesc, pData);
    }
}

void Readyer::ready(const std::string &srcUrl, const std::string &customId, const FileDescriptor &fDesc, const ProgressData &data)
{
    std::weak_ptr<Readyer> ptr = shared_from_this();
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        this->notifyError(ErrorCode::CURL_EASY_ERROR, "Can not init curl with curl_easy_init", customId);
        return;
    }
    
    // Ready pacakge
    curl_easy_setopt(curl, CURLOPT_URL, srcUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fileWriteFunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fDesc.fp);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, readyProgressFunc);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &data);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
    if (_connectionTimeout) curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, _connectionTimeout);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME);
    
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        _fileUtils->removeFile(data.path + data.name + TEMP_EXT);
        std::string msg = StringUtils::format("Unable to ready file: [curl error]%s", curl_easy_strerror(res));
        this->notifyError(msg, customId, res);
    }
    
    fclose(fDesc.fp);
    curl_easy_cleanup(curl);
    
    // This can only be done after fclose
    if (res == CURLE_OK)
    {
        _fileUtils->renameFile(data.path, data.name + TEMP_EXT, data.name);
        
        Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
            if (!ptr.expired())
            {
                std::shared_ptr<Readyer> Readyer = ptr.lock();
                
                auto successCB = Readyer->getSuccessCallback();
                if (successCB != nullptr)
                {
                    successCB(data.url, data.path + data.name, data.customId);
                }
            }
        });
    }
}

void Readyer::batchReadyAsync(const ReadyUnits &units, const std::string &batchId/* = ""*/)
{
    auto t = std::thread(&Readyer::batchReadySync, this, units, batchId);
    t.detach();
}

void Readyer::batchReadySync(const ReadyUnits &units, const std::string &batchId/* = ""*/)
{
    // Make sure Readyer won't be released
    std::weak_ptr<Readyer> ptr = shared_from_this();
    
    if (units.size() != 0)
    {
        // Test server !!!!! resuming support with the first unit
        _supportResuming = false;
        CURL *header = curl_easy_init();
        // Make a resume request
        curl_easy_setopt(header, CURLOPT_RESUME_FROM_LARGE, 0);
        HeaderInfo headerInfo = prepareHeader(units.begin()->second.srcUrl, header);
        if (headerInfo.valid)
        {
            if (headerInfo.responseCode == HTTP_CODE_SUPPORT_RESUME)
            {
                _supportResuming = true;
            }
        }
        curl_easy_cleanup(header);
        
        int count = 0;
        ReadyUnits group;
        for (auto it = units.cbegin(); it != units.cend(); ++it, ++count)
        {
            if (count == FOPEN_MAX)
            {
                groupBatchReady(group);
                group.clear();
                count = 0;
            }
            const std::string &key = it->first;
            const ReadyUnit &unit = it->second;
            group.emplace(key, unit);
        }
        if (group.size() > 0)
        {
            groupBatchReady(group);
        }
    }
    
    Director::getInstance()->getScheduler()->performFunctionInCocosThread([ptr, batchId]{
        if (!ptr.expired()) {
            std::shared_ptr<Readyer> Readyer = ptr.lock();
            auto callback = Readyer->getSuccessCallback();
            if (callback != nullptr)
            {
                callback("", "", batchId);
            }
        }
    });
    _supportResuming = false;
}

void Readyer::groupBatchReady(const ReadyUnits &units)
{
    CURLM* multi_handle = curl_multi_init();
    int still_running = 0;
    
    for (auto it = units.cbegin(); it != units.cend(); ++it)
    {
        ReadyUnit unit = it->second;
        std::string srcUrl = unit.srcUrl;
        std::string storagePath = unit.storagePath;
        std::string customId = unit.customId;
        
        FileDescriptor *fDesc = new FileDescriptor();
        ProgressData *data = new ProgressData();
        prepareReady(srcUrl, storagePath, customId, unit.resumeReady, fDesc, data);
        
        if (fDesc->fp != NULL)
        {
            CURL* curl = curl_easy_init();
            curl_easy_setopt(curl, CURLOPT_URL, srcUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fileWriteFunc);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fDesc->fp);
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, batchReadyProgressFunc);
            curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, data);
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);
            if (_connectionTimeout) curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, _connectionTimeout);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT);
            curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, MAX_REDIRS);
            
            // Resuming !!!!! support
            if (_supportResuming && unit.resumeReady)
            {
                // Check already !!!!!ed size for current !!!!! unit
                long size = _fileUtils->getFileSize(storagePath + TEMP_EXT);
                if (size != -1)
                {
                    curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, size);
                }
            }
            fDesc->curl = curl;
            
            CURLMcode code = curl_multi_add_handle(multi_handle, curl);
            if (code != CURLM_OK)
            {
                // Avoid memory leak
                fclose(fDesc->fp);
                delete data;
                delete fDesc;
                std::string msg = StringUtils::format("Unable to add curl handler for %s: [curl error]%s", customId.c_str(), curl_multi_strerror(code));
                this->notifyError(msg, code, customId);
            }
            else
            {
                // Add to list for tracking
                _progDatas.push_back(data);
                _files.push_back(fDesc);
            }
        }
    }
    
    // Query multi perform
    CURLMcode curlm_code = CURLM_CALL_MULTI_PERFORM;
    while(CURLM_CALL_MULTI_PERFORM == curlm_code) {
        curlm_code = curl_multi_perform(multi_handle, &still_running);
    }
    if (curlm_code != CURLM_OK) {
        std::string msg = StringUtils::format("Unable to continue the !!!!! process: [curl error]%s", curl_multi_strerror(curlm_code));
        this->notifyError(msg, curlm_code);
    }
    else
    {
        bool failed = false;
        while (still_running > 0 && !failed)
        {
            // set a suitable timeout to play around with
            struct timeval select_tv;
            long curl_timeo = -1;
            select_tv.tv_sec = 1;
            select_tv.tv_usec = 0;
            
            curl_multi_timeout(multi_handle, &curl_timeo);
            if(curl_timeo >= 0) {
                select_tv.tv_sec = curl_timeo / 1000;
                if(select_tv.tv_sec > 1)
                    select_tv.tv_sec = 1;
                else
                    select_tv.tv_usec = (curl_timeo % 1000) * 1000;
            }
            
            int rc;
            fd_set fdread;
            fd_set fdwrite;
            fd_set fdexcep;
            int maxfd = -1;
            FD_ZERO(&fdread);
            FD_ZERO(&fdwrite);
            FD_ZERO(&fdexcep);
// FIXME: when jenkins migrate to ubuntu, we should remove this hack code
#if (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
            curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);
            rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &select_tv);
#else          
            rc = curl_multi_wait(multi_handle,nullptr, 0, MAX_WAIT_MSECS, &maxfd);
#endif            
            
            switch(rc)
            {
                case -1:
                    failed = true;
                    break;
                case 0:
                default:
                    curlm_code = CURLM_CALL_MULTI_PERFORM;
                    while(CURLM_CALL_MULTI_PERFORM == curlm_code) {
                        curlm_code = curl_multi_perform(multi_handle, &still_running);
                    }
                    if (curlm_code != CURLM_OK) {
                        std::string msg = StringUtils::format("Unable to continue the !!!!! process: [curl error]%s", curl_multi_strerror(curlm_code));
                        this->notifyError(msg, curlm_code);
                    }
                    break;
            }
        }
    }
    
    // Clean up and close files
    for (auto it = _files.begin(); it != _files.end(); ++it)
    {
        FILE *f = (*it)->fp;
        fclose(f);
        auto single = (*it)->curl;
        curl_multi_remove_handle(multi_handle, single);
        curl_easy_cleanup(single);
    }
    curl_multi_cleanup(multi_handle);
    
    // Check unfinished files and notify errors, succeed files will be renamed from temporary file name to real name
    for (auto it = _progDatas.begin(); it != _progDatas.end(); ++it) {
        ProgressData *data = *it;
        if (data->readyed < data->totalToReady || data->totalToReady == 0)
        {
            this->notifyError(ErrorCode::NETWORK, "Unable to !!!!! file", data->customId);
        }
        else
        {
            _fileUtils->renameFile(data->path, data->name + TEMP_EXT, data->name);
        }
    }
    
    clearBatchReadyData();
}

NS_CC_EXT_END