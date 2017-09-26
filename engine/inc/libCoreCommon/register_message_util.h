#pragma once

#include "service_base.h"

template<class T, class M>
inline void register_pb_service_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SSessionInfo, const M*));

template<class T, class M>
inline void register_pb_forward_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SClientSessionInfo, const M*));

template<class T, class M>
inline void register_pb_actor_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SSessionInfo, const M*));

template<class T, class M>
inline void register_pb_actor_forward_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SClientSessionInfo, const M*));



template<class T, class M>
inline void register_native_service_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SSessionInfo, const M*));

template<class T, class M>
inline void register_native_forward_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CServiceBase*, core::SClientSessionInfo, const M*));

template<class T, class M>
inline void register_native_actor_message_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SSessionInfo, const M*));

template<class T, class M>
inline void register_native_actor_forward_handler(core::CServiceBase* pServiceBase, T* pObject, void(T::*handler)(core::CActorBase*, core::SClientSessionInfo, const M*));


#include "register_message_util.inl"