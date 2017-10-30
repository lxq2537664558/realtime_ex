#pragma once

#include "service_base.h"

template<class T, class M>
inline void register_service_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SSessionInfo, const M*));

template<class T, class M>
inline void register_forward_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SClientSessionInfo, const M*));

#include "register_message_util.inl"