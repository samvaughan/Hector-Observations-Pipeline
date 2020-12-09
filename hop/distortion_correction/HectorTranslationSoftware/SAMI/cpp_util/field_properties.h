#ifndef FIELD_PROPERTIES_INC
#define FIELD_PROPERTIES_INC

/*
 * "@(#) $Id: ACMM:cpp_util/field_properties.h,v 1.6 03-Nov-2016 08:24:43+11 tjf $"
 *
 * field_properties.h
 *
 * From "Imperfect C++", Chapter 35.  
 *
 *     
 * field_property_get           -> Provides a read-only property.
 * field_property_set           -> Provides a write-only property.
 * field_property_get_external  -> Provides a read-only property accessing
 *                                  an existing member variable.
 * field_property_set_external  -> Provides a write-only property accessing
 *                                  an existing member variable.
 *
 * Author: Matthew Wilson.
 *
 * History:
 *     16-May-2006 - TJF - Copied and commenced commenting etc.
 *     11-Sep-2006 - TJF - Added FIELD_PROPERTIES_INC  def.
 *     07-Jul-2015 - TJF - Use PMDIAG_CPP11_EXT*  macros to stop CLang warnings
 *                           about declaring template as friend being a c++11
 *                           extension.
 */
#include "port_macros.h"
namespace  aao_cpp_util {

/** Declare a read only property.
 *
 * For example, to create a read-only property named "Count", of type
 * int in class X, you declare it as follows.
 * <pre>
 * class X {...
 *     field_property_get<int, int, X>  Count;
 * </pre>
 * Class X can change the value using Count.m_value.
 *
 * External classes can access using use "Count".
 *
 * @param V This must be the type of the properity.
 * @param R The type of the property value reference - what is returned.
 * @param C The enclosing class.
 */

template< typename V   /* The actual property value type */
        , typename R   /* The reference type */
        , typename C   /* The enclosing class */
    >
class field_property_get
{
public:
  typedef field_property_get<V, R, C> class_type;

private:
  // Does not initialise m_value
  field_property_get()
  {}
  // Initialises m_value to given value
  explicit field_property_get(R value)
    : m_value(value)
  {}

PMDIAG_CPP11_EXT_OFF   // Stops warnings in CLang. 
  DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);
PMDIAG_CPP11_EXT_ON

public:
  /// Provides read-only access to the property 
  operator R () const
  {
    return m_value;
  }

private:
  V m_value;

// Not to be implemented
private:
  field_property_get(class_type const &);
  class_type &operator =(class_type const &);
};




/** Declare a write only property.
 *
 * For example, to create a write-only property named "Count", of type
 * int in class X, you declare it as follows.
 * <pre>
 * class X {...
 *     field_property_set<int, int, X>  Count;
 * </pre>
 * Class X can read/write the value using Count.m_value.
 *
 * External classes and write this using "Count = <value>";
 *
 * @param V This must be the type of the properity.
 * @param R The type of the property value reference - what is returned.
 * @param C The enclosing class.
 */

template< typename V   /* The actual property value type */
        , typename R   /* The reference type */
        , typename C   /* The enclosing class */
    >
class field_property_set
{
public:
  typedef field_property_set<V, R, C> class_type;

private:
  field_property_set()
  {}
  explicit field_property_set(R value)
    : m_value(value)
  {}

PMDIAG_CPP11_EXT_OFF   // Stops warnings in CLang. 
    DECLARE_TEMPLATE_PARAM_AS_FRIEND(C);
PMDIAG_CPP11_EXT_ON

public:
  /// Provides write-only access to the property
  class_type &operator =(R value)
  {
    m_value = value;

    return *this;
  }

private:
  V  m_value;

// Not to be implemented
private:
  field_property_set(class_type const &);
  class_type &operator =(class_type const &);
};


/** Declare a read only property referencing an existing member.
 *
 * For example, to create a read-only property named "Count", of type
 * int in class X, you declare it as follows.
 * <pre>
 * class X {...
 *     Rectangle(...) : Count(m_count);
 *     field_property_get<int, int>  Count;
 * </pre>
 * External classes can access using use "Count".
 *
 * @param V This must be the type of the properity.
 * @param R The type of the property value reference - what is returned.
 */

template< typename V   /* The actual property value type */
        , typename R   /* The reference type */
    >
class field_property_get_external
{
public:
  field_property_get_external(V &value)
    : m_value(value)
  {}

// Accessors
public:
  /// Provides read-only access to the property
  operator R() const
  {
    return m_value;
  }

// Members
private:
  V  &m_value;
};

/** Declare a write only property referencing an existing member.
 *
 * For example, to create a read-only property named "Count", of type
 * int in class X, you declare it as follows.
 * <pre>
 * class X {...
 *     Rectangle(...) : Count(m_count);
 *     field_property_get<int, int>  Count;
 * </pre>
 * External classes can write using use "Count=<val>".
 *
 * @param V This must be the type of the properity.
 * @param R The type of the property value reference - what is returned.
 */
template< typename V   /* The actual property value type */
        , typename R   /* The reference type */
    >
class field_property_set_external
{
public:
  field_property_set_external(V &value)
    : m_value(value)
  {}

// Accessors
public:
  /// Provides write-only access to the property
  field_property_set_external &operator =(R value)
  {
    m_value = value;

    return *this;
  }

// Members
private:
  V  &m_value;
};

} // namespace aao_cpp_util
#endif /* !def FIELD_PROPERTIES_INC */
