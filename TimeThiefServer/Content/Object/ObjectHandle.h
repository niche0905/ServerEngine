#pragma once
#include "ObjectId.h"
#include "ObjectManager.h"

struct ObjectId;
class ObjectManager;
class BaseObject;

/*----------------
   ObjectHandle
----------------*/
//
// ObjectHandle은 대상을 Pointer로 직접 참조하지 않고 간접적으로 참조하는 핸들입니다.
// 서버에서 흔한 유스 애프터 프리 버그를 방지하기 위해 사용됩니다.
//

class ObjectHandle
{
public:
   ObjectHandle() = default;
   explicit ObjectHandle(ObjectId id) : id_(id) {}
   
   ObjectId GetId() const { return id_; }
   bool IsValid() const { return static_cast<bool>(id_); }
   
   BaseObject* Resolve(ObjectManager& om) const
   {
      return om.Find(id_);
   }
   
   template<typename T>
   T* ResolveAs(ObjectManager& om) const
   {
      static_assert(std::is_base_of_v<BaseObject, T>);
      return om.FindAs<T>(id_);
   }
   
   void Reset() { id_ = {}; }
   
   bool operator==(const ObjectHandle& rhs) const { return id_ == rhs.id_; }
   bool operator!=(const ObjectHandle& rhs) const { return id_ != rhs.id_; }
   
   explicit operator bool() const { return IsValid(); }
   
private:
   ObjectId id_{};
   
};