//#####################################################################
// Class Value
//#####################################################################
#pragma once

#include <other/core/value/forward.h>
#include <other/core/python/Object.h>
#include <other/core/python/try_convert.h>
#include <other/core/python/ExceptionValue.h>
#include <other/core/utility/Optional.h>
#include <other/core/vector/Vector.h>
#include <boost/type_traits/is_const.hpp>
#include <boost/type_traits/is_reference.hpp>
extern void wrap_value_base();

#include <other/core/python/Ptr.h>
#include <vector>

namespace other {

using std::vector;
using std::type_info;
using boost::is_const;
using boost::is_reference;

class OTHER_CORE_CLASS_EXPORT ValueBase : public Object {
public:
  OTHER_DECLARE_TYPE(OTHER_CORE_EXPORT)
  typedef Object Base;
private:
  friend class Action;
  template<class T> friend class Value;
  mutable bool dirty_; // are we up to date?
  mutable ExceptionValue error; // is the value an exception?
public:
  string name;
private:

  // A link between an Action and a Value
  struct Link
  {
    Link **value_prev, *value_next; // pointers for the value's linked list
    Link **action_prev, *action_next; // pointers for the action's linked list
    const ValueBase* value; // used only by dump()
    Action* action;
  };
  mutable Link* actions; // linked list of links to actions which depend on us
  static Link* pending; // linked list of pending signals

protected:
  OTHER_CORE_EXPORT ValueBase();
public:
  OTHER_CORE_EXPORT virtual ~ValueBase();

#ifdef OTHER_PYTHON
  virtual PyObject* get_python() const = 0;

  PyObject* operator()() const {
    return get_python();
  }
#endif

  bool is_prop() const;
  virtual const type_info& type() const = 0;

  bool dirty() const {
    return dirty_;
  }

  template<class T> const Value<T>* cast() const {
    return is_type(typeid(T)) ? static_cast<const Value<T>*>(this) : 0;
  }

  OTHER_CORE_EXPORT bool is_type(const type_info& type) const;
  OTHER_CORE_EXPORT void signal() const;

  virtual void dump(int indent) const = 0;
  virtual vector<Ref<const ValueBase>> dependencies() const = 0;

  const string& get_name() const;
  OTHER_CORE_EXPORT ValueBase& set_name(const string& n);

private:
  OTHER_CORE_EXPORT void pull() const;

  virtual void update() const = 0;
  static inline void signal_pending();

  // The following exist only for python purposes: they throw exceptions if the ValueBase isn't a PropBase.
  PropBase& prop();
  const PropBase& prop() const;
  ValueBase& set_help(const string& h);
  ValueBase& set_category(const string& c);
  ValueBase& set_hidden(bool h);
  ValueBase& set_required(bool r);
  ValueBase& set_abbrev(char a);
  const string& get_help() const;
  const string& get_category() const;
  bool get_hidden() const;
  bool get_required() const;
  char get_abbrev() const;
  friend void ::wrap_value_base();
#ifdef OTHER_PYTHON
  ValueBase& set(PyObject* value_);
  ValueBase& set_allowed(PyObject* v);
  ValueBase& set_min_py(PyObject* m);
  ValueBase& set_max_py(PyObject* m);
  ValueBase& set_step_py(PyObject* s);
  PyObject* get_default() const;
  PyObject* get_allowed() const;
#endif
};

template<class T> class OTHER_CORE_CLASS_EXPORT Value : public ValueBase
{
  static_assert(!is_const<T>::value,"T can't be const");
  static_assert(!is_reference<T>::value,"T can't be a reference");
  static_assert(!boost::is_same<T,char*>::value && !boost::is_same<T,const char*>::value,"T can't be char*");
public:
  typedef ValueBase Base;
    typedef T ValueType;

protected:
  mutable Optional<T> value; // the cached value

  Value() {}
  ~Value() {}

  void set_dirty() const {
    if (!dirty_) {
      dirty_ = true;
      error = ExceptionValue();
      value.reset();
      signal();
    }
  }

  void set_value(const T& value_) const {
    dirty_ = false;
    error = ExceptionValue();
    value = value_;
    signal();
  }

public:
  const T& operator()() const {
    pull();
    return *value;
  }

#ifdef OTHER_PYTHON
  PyObject* get_python() const {
    pull();
    return try_to_python(*value);
  }
#endif

  const type_info& type() const {
    return typeid(T);
  }

  // Look at a value without adding a dependency graph node
  const T& peek() const {
    OTHER_ASSERT(!dirty_);
    return *value;
  }
};

template<class T> class ValueRef
{
  static_assert(!is_const<T>::value,"T can't be const");
  static_assert(!is_reference<T>::value,"T can't be a reference");
public:
  typedef T ValueType;

  Ref<const Value<T>> self;

  ValueRef(const Value<T>& value)
    : self(other::ref(value)) {}

  ValueRef(const PropRef<T>& prop)
    : self(prop.self) {}

  ValueRef(const PropRef<const T>& prop)
    : self(prop.self) {}

  const T& operator()() const {
    return (*self)();
  }

  const Value<T>& operator*() const {
    return *self;
  }

  const Value<T>* operator->() const {
    return &*self;
  }

  operator const Value<T>&() const {
    return *self;
  }
};

template<class T> static inline PyObject* to_python(const ValueRef<T>& value) {
  return to_python(*value.self);
}

// from_python<ValueRef<T>> is declared in convert.h to avoid circularity

}
