LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := cocos_curl_static

LOCAL_MODULE_FILENAME := libcurl

LOCAL_SRC_FILES := lib/altsvc.c \
lib/amigaos.c \
lib/asyn-ares.c \
lib/asyn-thread.c \
lib/base64.c \
lib/conncache.c \
lib/connect.c \
lib/content_encoding.c \
lib/cookie.c \
lib/curl_addrinfo.c \
lib/curl_ctype.c \
lib/curl_des.c \
lib/curl_endian.c \
lib/curl_fnmatch.c \
lib/curl_get_line.c \
lib/curl_gethostname.c \
lib/curl_gssapi.c \
lib/curl_memrchr.c \
lib/curl_multibyte.c \
lib/curl_ntlm_core.c \
lib/curl_ntlm_wb.c \
lib/curl_path.c \
lib/curl_range.c \
lib/curl_rtmp.c \
lib/curl_sasl.c \
lib/curl_sspi.c \
lib/curl_threads.c \
lib/dict.c \
lib/doh.c \
lib/dotdot.c \
lib/easy.c \
lib/escape.c \
lib/file.c \
lib/fileinfo.c \
lib/formdata.c \
lib/ftp.c \
lib/ftplistparser.c \
lib/getenv.c \
lib/getinfo.c \
lib/gopher.c \
lib/hash.c \
lib/hmac.c \
lib/hostasyn.c \
lib/hostcheck.c \
lib/hostip.c \
lib/hostip4.c \
lib/hostip6.c \
lib/hostsyn.c \
lib/http_chunks.c \
lib/http_digest.c \
lib/http_negotiate.c \
lib/http_ntlm.c \
lib/http_proxy.c \
lib/http.c \
lib/http2.c \
lib/idn_win32.c \
lib/if2ip.c \
lib/imap.c \
lib/inet_ntop.c \
lib/inet_pton.c \
lib/krb5.c \
lib/ldap.c \
lib/llist.c \
lib/md4.c \
lib/md5.c \
lib/memdebug.c \
lib/mime.c \
lib/mprintf.c \
lib/multi.c \
lib/netrc.c \
lib/non-ascii.c \
lib/nonblock.c \
lib/nwlib.c \
lib/nwos.c \
lib/openldap.c \
lib/parsedate.c \
lib/pingpong.c \
lib/pop3.c \
lib/progress.c \
lib/psl.c \
lib/rand.c \
lib/rtsp.c \
lib/security.c \
lib/select.c \
lib/sendf.c \
lib/setopt.c \
lib/sha256.c \
lib/share.c \
lib/slist.c \
lib/smb.c \
lib/smtp.c \
lib/socks_gssapi.c \
lib/socks_sspi.c \
lib/socks.c \
lib/speedcheck.c \
lib/splay.c \
lib/ssh-libssh.c \
lib/ssh.c \
lib/strcase.c \
lib/strdup.c \
lib/strerror.c \
lib/strtok.c \
lib/strtoofft.c \
lib/system_win32.c \
lib/telnet.c \
lib/tftp.c \
lib/timeval.c \
lib/transfer.c \
lib/url.c \
lib/urlapi.c \
lib/version.c \
lib/warnless.c \
lib/wildcard.c \
lib/x509asn1.c \
lib/vauth/cleartext.c \
lib/vauth/cram.c \
lib/vauth/digest_sspi.c \
lib/vauth/digest.c \
lib/vauth/krb5_gssapi.c \
lib/vauth/krb5_sspi.c \
lib/vauth/ntlm_sspi.c \
lib/vauth/ntlm.c \
lib/vauth/oauth2.c \
lib/vauth/spnego_gssapi.c \
lib/vauth/spnego_sspi.c \
lib/vauth/vauth.c \
lib/vtls/cyassl.c \
lib/vtls/gskit.c \
lib/vtls/gtls.c \
lib/vtls/mbedtls.c \
lib/vtls/mesalink.c \
lib/vtls/nss.c \
lib/vtls/openssl.c \
lib/vtls/polarssl_threadlock.c \
lib/vtls/polarssl.c \
lib/vtls/schannel_verify.c \
lib/vtls/schannel.c \
lib/vtls/sectransp.c \
lib/vtls/vtls.c 

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include/android \
$(LOCAL_PATH)/lib


LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/android \
$(LOCAL_PATH)/lib

LOCAL_STATIC_LIBRARIES += cocos_crypto_static

include $(BUILD_STATIC_LIBRARY)
