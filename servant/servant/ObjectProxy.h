﻿/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */

#ifndef __TARS_OBJECT_PROXY_H_
#define __TARS_OBJECT_PROXY_H_

#include "servant/Communicator.h"
#include "servant/Message.h"
#include "servant/AdapterProxy.h"
#include "servant/EndpointInfo.h"
#include "servant/EndpointF.h"
#include "servant/AppProtocol.h"
#include "servant/StatReport.h"
#include "servant/Global.h"
#include "util/tc_timeout_queue_noid.h"

namespace tars
{
class EndpointManager;

///////////////////////////////////////////////////////////////////
/**
 * socket选项
 */
struct SocketOpt
{
    int        level;

    int        optname;

    const void *optval;

    SOCKET_LEN_TYPE  optlen;
};

///////////////////////////////////////////////////////////////////
/**
 * 每个objectname在每个客户端网络线程中有唯一一个objectproxy
 * 
 */
class ObjectProxy : public TC_HandleBase, public TC_ThreadMutex
{
public:
    /**
     * 构造函数
     * @param pCommunicatorEpoll
     * @param sObjectProxyName
     * @param setName 指定set调用的setid
     */
    ObjectProxy(CommunicatorEpoll * pCommunicatorEpoll, const string & sObjectProxyName, const string& setName="");

    /**
     * 析构函数
     */
    ~ObjectProxy();

    /**
     * 初始化
     */
    void initialize();

    /**
     * 加载locator
     */
    int loadLocator();

    /**
     * 方法调用
     */
    void invoke(ReqMessage* msg);

    /**
     * 发送积攒在obj的数据
     */
    void doInvoke();

    /**
     * 设置协议解析器
     * @return UserProtocol&
     */
    void setProxyProtocol(const ProxyProtocol& protocol);

    /**
     * 获取协议解析器
     * @return ProxyProtocol&
     */
    ProxyProtocol& getProxyProtocol();

    /**
     * 设置套接口选项
     */
    void setSocketOpt(int level, int optname, const void *optval, socklen_t optlen);

    /**
     * 获取套接字选项
     */
    vector<SocketOpt>& getSocketOpt();

    /**
     * 设置PUSH类消息的callback对象
     * @param cb
     */
    void setPushCallbacks(const ServantProxyCallbackPtr& cb);

    /**
     * 获取PUSH类消息的callback对象
     */
    ServantProxyCallbackPtr getPushCallback();

    /**
     * 获取所有的adapter
     */
    // const vector<AdapterProxy*> & getAdapters() const;

    void mergeStat(map<StatMicMsgHead, StatMicMsgBody> & mStatMicMsg);

    /**
     * 检查Obj的队列超时
     */
    void doTimeout();

    /**
     * Obj的超时队列的长度
     */
    size_t timeoutQSize()
    {
        return _reqTimeoutQueue.size();
    }
    
    /**
     * 获取CommunicatorEpoll*
     */
    inline CommunicatorEpoll * getCommunicatorEpoll()
    {
        return _communicatorEpoll;
    }

    /**
     * 获取object名称
     * @return const string&
     */
    inline const string & name() const
    {
        return _name;
    }

    /**
     * 判断此obj是否走按set规则调用流程，如果是直连方式，即使服务端是启用set的，也不认为是按set规则调用的
     */
    bool isInvokeBySet() const
    {
        return _isInvokeBySet;
    }

    /**
     * 获取按set规则调用的set名称
     */
    const string& getInvokeSetName() const
    {
        return _invokeSetId;
    }

    /**
     * 获取连接超时时间
     * @return int
     */
    inline int getConTimeout()
    {
        return _conTimeout;
    }

    /**
     * 设置连接超时时间
     */
    inline void setConTimeout(int conTimeout)
    {
        _conTimeout = conTimeout;
    }

    /**
     * 超时策略获取和设置
     * @return CheckTimeoutInfo&
     */
    inline CheckTimeoutInfo& checkTimeoutInfo()
    {
        return _checkTimeoutInfo;
    }

    /**
     * 获取servantproxy
     */
    inline ServantProxy * getServantProxy()
    {
        return _servantProxy;
    }

    /**
     * 设置servantproxy
     */
    inline void setServantProxy(ServantProxy * pServantProxy)
    {
        _servantProxy = pServantProxy;
    }

protected:

    /**
     * 处理请求异常
     */
    void doInvokeException(ReqMessage * msg);

private:
    /*
     * 客户端网络线程的指针
     */
    CommunicatorEpoll *                   _communicatorEpoll;

    /*
     * object的名称
     */
    string                                _name;

    /*
     * 按set规则调用的set名称
     */
    string                                _invokeSetId;

    /*
     * 是否是按set规则调用
     */
    bool                                  _isInvokeBySet;

    /*
     * 是否调用了tars_set_protocol设置过proxy的协议函数，
     * 设置过了就不在设置
     */
    bool                                  _hasSetProtocol;

    /*
     * 请求和响应的协议解析器
     */
    ProxyProtocol                         _proxyProtocol;

    /*
     * 连接超时的时间
     */
    int                                   _conTimeout;

    /*
     * 超时控制策略信息
     */
    CheckTimeoutInfo                      _checkTimeoutInfo;

    /*
     * socket选项
     */
    vector<SocketOpt>                     _socketOpts;

    /*
     * push消息 callback
     */
    ServantProxyCallbackPtr               _pushCallback;

    /*
     * 结点路由管理类
     */
    std::unique_ptr<EndpointManager>      _endpointManger;

    /*
     * 超时队列
     */
    TC_TimeoutQueueNoID<ReqMessage *>     _reqTimeoutQueue;

    /*
     * ServantProxy
     */
    ServantProxy *                        _servantProxy;
};
///////////////////////////////////////////////////////////////////////////////////
}
#endif
