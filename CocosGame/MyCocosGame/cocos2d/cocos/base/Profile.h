///////////////////////////////////////////////////////////////////////////////////////////////////
//
// 文件名	profile.h
//
// 模块名	实时性能评估
//
// 作者		Greg Hjelstrom & Byon Garrabrant
// 地址      http://multi-crash.com/?page_id=174
// 历史		日期		姓名		描述
//
//			03_11_20	郑榕		修改
//			04_11_12	郑榕		修改	增加pfl文件的版本标识，从此不再兼容旧版本的pfl文件
//			06_02_11	郑榕		修改
//			07_02_26	郑榕		修改	添加CProfileIterator::SetParent接口
//          11_03_15    刘思阳   增加了多线程支持，使用的是TLS
//          19_03_18    刘思阳   使用了std的chrono来进行计时，这样移植方便些
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

//#define _PERFORMANCE

// 采样
#ifdef _PERFORMANCE

// 其实做个策略模式可能更好。暂时懒得修改了
//#define USE_C_CLOCK
#ifdef USE_C_CLOCK
#include <ctime>
#define CLOCK_TYPE clock_t
inline CLOCK_TYPE PflGetTicks()
{
    return clock();
}
inline double ClockDiffToTime(CLOCK_TYPE& end_ticks, CLOCK_TYPE& begin_ticks )
{
    return ((double)(end_ticks-begin_ticks))/CLOCKS_PER_SEC;
}
#else
#include <chrono>
#define CLOCK_TYPE std::chrono::system_clock::time_point
inline CLOCK_TYPE PflGetTicks()
{
    return std::chrono::system_clock::now();
}
inline double ClockDiffToTime(CLOCK_TYPE& end_ticks, CLOCK_TYPE& begin_ticks )
{
    std::chrono::duration<double> diff = end_ticks-begin_ticks;
    return diff.count();
}
#endif




struct PROFILE_FILE_HEADER;    //pfl文件头
struct PROFILE_NODE_HEADER;    //pfl文件采样节点结构
struct PROFILE_NODE_DATA;      //pfl文件采样节点数据

///////////////////////////////////////////////////////////////////////////////////////////////////
// 性能评估树节点
///////////////////////////////////////////////////////////////////////////////////////////////////
class CProfileNode
{
public:
    CProfileNode( const char* name, CProfileNode *parent );
    CProfileNode( const char* name );
    ~CProfileNode();
    
    void Reset();
    inline void Call();
    inline bool Return();
    
    CProfileNode* GetParent()
    {
        return Parent;
    }
    CProfileNode* GetSibling()
    {
        return Sibling;
    }
    CProfileNode* GetChild()
    {
        return Child;
    }
    const char* GetName()
    {
        return Name;
    }
    int GetIndex()
    {
        return iIndex;
    }
    int	GetTotalCalls()
    {
        return TotalCalls;
    }
    double GetTotalTime()
    {
        return TotalTime;
    }
    
protected:
    const char*		Name;				//节点名
    int				TotalCalls;			//总调用次数
    double			TotalTime;			//总开销时间
    CLOCK_TYPE		StartTime;			//最后调用时间
    int				RecursionCounter;	//递归计数
    
    CProfileNode*	Parent;
    CProfileNode*	Child;
    CProfileNode*	Sibling;
    
    //节点序号，为了存盘时候的方便
    int				iIndex;
    
    friend	class	CProfileManager;
};

inline void CProfileNode::Call()
{
    TotalCalls++;
    if ( RecursionCounter++ == 0 )
    {
        StartTime = PflGetTicks();
    }
}

inline bool CProfileNode::Return()
{
    if ( --RecursionCounter == 0 && TotalCalls != 0 )
    {
        CLOCK_TYPE end_time = PflGetTicks();
        TotalTime += ClockDiffToTime(end_time, StartTime);
//        time -= StartTime;
//        TotalTime += time;
    }
    
    return ( RecursionCounter == 0 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 评估树浏览器
///////////////////////////////////////////////////////////////////////////////////////////////////
class CProfileIterator
{
public:
    // Access all the children of the current parent
    CProfileNode* First();
    CProfileNode* Next();
    
    bool IsDone();
    void EnterChild( int index );	// Make the given child the new parent
    void EnterLargestChild();		// Make the largest child the new parent
    void EnterParent();				// Make the current parent's parent the new parent
    int GetTotalChild();			//获取当前parent的child个数
    
    //zr 07_02_26	将指定节点设置为当前父节点
    void SetParent( CProfileNode* pNode );
    
    // Access the current child
    const char* GetCurrentName()
    {
        return CurrentChild->GetName();
    }
    int	GetCurrentTotalCalls()
    {
        return CurrentChild->GetTotalCalls();
    }
    double GetCurrentTotalTime()
    {
        return CurrentChild->GetTotalTime();
    }
    
    const char* GetCurrentParentName()
    {
        return CurrentParent->GetName();
    }
    int GetCurrentParentTotalCalls()
    {
        return CurrentParent->GetTotalCalls();
    }
    double GetCurrentParentTotalTime()
    {
        return CurrentParent->GetTotalTime();
    }
    
    CProfileNode* Parent()
    {
        return CurrentParent;
    }
    
protected:
    CProfileNode*	CurrentParent;
    CProfileNode*	CurrentChild;
    
    CProfileIterator( CProfileNode* start );
    friend	class	CProfileManager;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//评估系统管理器
///////////////////////////////////////////////////////////////////////////////////////////////////
class CProfileManager
{
private:
    CProfileManager();
    ~CProfileManager();
public:
    static CProfileManager* createTheOne();
    static void destroyTheOne();
    
    static void setTheOne( CProfileManager* perfManager );
    static CProfileManager* getTheOne();
    
public:
    // 多线程的性能评估
    static int createThreadPerf();
    static void destroyThreadPerf();
    static CProfileManager* getThreadPerf();
    
public:
    inline void StartProfile( const char* name );
    inline void StopProfile();
    inline CProfileNode* FindSubNode( CProfileNode* pParent, const char* name );
    
    CProfileNode* CreateNewNode( const char* name, CProfileNode *pParent );
    
    void Reset();
    void IncrementFrameCounter();
    int GetFrameCountSinceReset()
    {
        return FrameCounter;
    }
    double	GetTimeSinceReset();
    
    CProfileIterator* GetIterator()
    {
        return new CProfileIterator( m_pRoot );
    }
    void ReleaseIterator( CProfileIterator * iterator )
    {
        delete iterator;
    }
    
    double GetRootTotalTime();
    
    
    //////////////////////////////////////////////////////////////////////////////
    //以下为存盘/读盘相关接口
    //////////////////////////////////////////////////////////////////////////////
    bool OpenDataFile( const char* pPath, int* pTotalDataFrame, int* pUserDataSize );
    bool CreateFileHeader( const char* pPath, int iUserDataSize, const char** pUserDataName );
    bool RecreateDataFile( const char* pPath, int iTotalNodeOld,
                          int iUserDataSize,	const char** pUserDataName );
    void ExportNodeHeader( CProfileNode *pNode, PROFILE_NODE_HEADER* pHeader );
    void ExportNodeData( CProfileNode *pNode, PROFILE_NODE_DATA* pData );
    
    bool SaveDataFrame( const char* pPath, int* pUserData, int iSize, const char** pUserDataName );
    bool LoadDataFrame( const char* pPath, int iFrameIndex, int* pUserData, long *pTime );
    
    bool GetUserDataName( const char* pPath, char* pName );
    
    bool GetNodeTimeArr( const char* pPath, int iNodeIndex,
                        double* pTime, double* pTimePerFr,
                        double& iMaxTime, double& iMinTime,
                        int& iMaxIndex, int& iMinIndex, double& iMaxTimePerFr );
    
    bool GetUserDataArr( const char* pPath, int iIndex, int* pData,
                        int& iMaxValue, int& iMinValue,
                        int& iMaxIndex, int& iMinIndex );
    
    bool GetTotalFrame( const char* pPath, int& iTotalFrame,
                       int iFrameBegin, int iFrameEnd );
    bool GetNodeTotalData( const char* pPath, int iNodeIndex, double& i64TotalTime, int& iTotalCall,
                          int iFrameBegin, int iFrameEnd );
    bool GetTotalUserDataArr( const char* pPath, double* pTotalUserData,
                             int iFrameBegin, int iFrameEnd );
    
private:
    void ReleaseMemory();
    bool VerifyFileHeader( PROFILE_FILE_HEADER* pHeader );
    
    bool HandleOldFile( const char* pPath, int iUserDataSize, const char** pUserDataName );
    bool copyFile(const char *src, const char *des);
    
public:
    int				iTotalNode;	//总节点数, added by zr 04_02_25
    
private:
    CProfileNode*	m_pRoot;
    CProfileNode*	m_pCurrentNode;
    int				FrameCounter;
    CLOCK_TYPE      ResetTime;
    
    static CProfileManager* theOne;
    
    //以下只有ProfileViewer才用
    CProfileNode**			m_pNodeArr;
    PROFILE_NODE_HEADER*	m_pNodeData;
};

inline void CProfileManager::StartProfile( const char* name )
{
    //使用指针比较取代字符串比较
    if ( name != m_pCurrentNode->GetName() )	//如果相等就是递归
    {	//非递归
        m_pCurrentNode = FindSubNode( m_pCurrentNode, name );
    }
    m_pCurrentNode->Call();
}

inline void CProfileManager::StopProfile()
{
    //如果没有返回true, 则说明采样的函数块被重入(递归调用)了
    if ( m_pCurrentNode->Return() )
    {
        m_pCurrentNode = m_pCurrentNode->GetParent();
    }
}

inline CProfileNode* CProfileManager::FindSubNode( CProfileNode* pParent, const char* name )
{
    // Try to find this sub node
    CProfileNode * child = pParent->GetChild();
    while ( child )
    {
        if ( child->GetName() == name )
        {
            return child;
        }
        child = child->GetSibling();
    }
    // We didn't find it, so add it
    return CreateNewNode( name, pParent );
}

/*
 ** ProfileSampleClass is a simple way to profile a function's scope
 ** Use the PROFILE macro at the start of scope to time
 */
class CPerfSample
{
public:
    CPerfSample( const char* name );
    ~CPerfSample();
};

class CThreadPerfSample
{
public:
    CThreadPerfSample( const char* name );
    ~CThreadPerfSample();
};


#define MERGE_STR(x, y) x##y
#define LABEL_STR(x, y) MERGE_STR(x, y)
#define UNIQUE_NAME LABEL_STR(perf_, __LINE__)


#define	PERF_NODE( name )		CPerfSample UNIQUE_NAME( name );
#define PERF_NODE_FUNC()		CPerfSample UNIQUE_NAME( __FUNCTION__ );
#define	THREAD_PERF_NODE( name )		CThreadPerfSample UNIQUE_NAME( name )
#define THREAD_PERF_NODE_FUNC()			CThreadPerfSample UNIQUE_NAME( __FUNCTION__ )

#else

#define	PERF_NODE( name )
#define PERF_NODE_FUNC()
#define	THREAD_PERF_NODE( name )
#define THREAD_PERF_NODE_FUNC()

#endif
