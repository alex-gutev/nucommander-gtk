/**
 * Definition of Glib::Variant<std::tuple<...>>.
 */

#ifndef NUC_SETTINGS_VARIANT_TUPLE_H
#define NUC_SETTINGS_VARIANT_TUPLE_H


/* Copyright 2010 The glibmm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <tuple>

namespace Glib {

/** Specialization of Variant containing a tuple.
 * @newin{2,54}
 * @ingroup Variant
 */
template <class... Types>
class Variant<std::tuple<Types...>> : public VariantContainerBase
{
public:
  using CppContainerType = std::tuple<Types...>;

  /// Default constructor
  Variant<std::tuple<Types...>>()
  : VariantContainerBase()
  {}

  /** GVariant constructor.
   * @param castitem The GVariant to wrap.
   * @param take_a_reference Whether to take an extra reference of the GVariant
   *        or not (not taking one could destroy the GVariant with the wrapper).
   */
  explicit Variant<std::tuple<Types...>>(GVariant* castitem, bool take_a_reference = false)
  : VariantContainerBase(castitem, take_a_reference)
  {}

  /** Creates a new Variant containing a tuple.
   * @param data The tuple to use for creation.
   * @return The new Variant holding a tuple.
   * @newin{2,54}
   */
  static Variant<std::tuple<Types...>> create(const std::tuple<Types...>& data);

  /** Gets the VariantType.
   * @return The VariantType.
   * @newin{2,54}
   */
  static const VariantType& variant_type() G_GNUC_CONST;

  /** Gets a specific element from the tuple.
   * It is an error if @a index is greater than or equal to the number of
   * elements in the tuple. See VariantContainerBase::get_n_children().
   *
   * @param index The index of the element.
   * @return The tuple element at index @a index.
   * @throw std::out_of_range
   * @newin{2,54}
   */
  template<class T>
  T get_child(gsize index) const;

  template<class T>
  Variant<T> get_child_variant(gsize index) const;

  /** Gets the tuple of the Variant.
   * @return The tuple.
   * @newin{2,54}
   */
  std::tuple<Types...> get() const;

  /** Gets a VariantIter of the Variant.
   * @return The VariantIter.
   * @newin{2,54}
   */
  VariantIter get_iter() const;
};

/*---------------------Variant<std::tuple<class... Types>> --------------------*/

// static
template <class... Types>
const VariantType& Variant<std::tuple<Types...>>::variant_type()
{
  std::vector<VariantType> types;
  auto expander = [&types](const VariantType &type) mutable -> int
  {
    types.push_back(type);
    return 0;
  };

  // expands the variadic template parameters
  using swallow = int[]; // ensures left to right order
  (void)swallow{(expander(Variant<Types>::variant_type()))...};
  static auto type = VariantType::create_tuple(types);

  return type;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace detail
{
// std::index_sequence and std::index_sequence_for are new in C++14,
// but this version of glibmm requires only C++11.
// The following code replaces std::index_sequence and std::index_sequence_for
// until we can require C++14 support.
// See https://bugzilla.gnome.org/show_bug.cgi?id=787648

  /// Class template integer_sequence
  template<typename T, T... Idx>
    struct integer_sequence
    {
      typedef T value_type;
      static constexpr std::size_t size() { return sizeof...(Idx); }
    };

  // Concatenates two integer_sequences.
  template<typename Iseq1, typename Iseq2> struct iseq_cat;

  template<typename T, std::size_t... Ind1, std::size_t... Ind2>
    struct iseq_cat<integer_sequence<T, Ind1...>, integer_sequence<T, Ind2...>>
    {
      using type = integer_sequence<T, Ind1..., (Ind2 + sizeof...(Ind1))...>;
    };

  // Builds an integer_sequence<T, 0, 1, 2, ..., Num-1>.
  template<typename T, std::size_t Num>
    struct make_intseq
    : iseq_cat<typename make_intseq<T, Num / 2>::type,
		typename make_intseq<T, Num - Num / 2>::type>
    { };

  template<typename T>
    struct make_intseq<T, 1>
    {
      typedef integer_sequence<T, 0> type;
    };

  template<typename T>
    struct make_intseq<T, 0>
    {
      typedef integer_sequence<T> type;
    };

  /// Alias template make_integer_sequence
  template<typename T, T Num>
    using make_integer_sequence = typename make_intseq<T, Num>::type;

  /// Alias template index_sequence
  template<std::size_t... Idx>
    using index_sequence = integer_sequence<std::size_t, Idx...>;

  /// Alias template make_index_sequence
  template<std::size_t Num>
    using make_index_sequence = make_integer_sequence<std::size_t, Num>;

  /// Alias template index_sequence_for
  template<typename... Types>
    using index_sequence_for = make_index_sequence<sizeof...(Types)>;

// End of code that replaces std::index_sequence and std::index_sequence_for

template <class Tuple, std::size_t... Is>
void expand_tuple(std::vector<VariantBase> &variants, const Tuple & t,
                  detail::index_sequence<Is...>)
{
  using swallow = int[]; // ensures left to right order
  auto expander = [&variants](const VariantBase &variant) -> int
  {
    variants.push_back(variant);
    return 0;
  };
  (void)swallow {(expander(Variant<typename std::tuple_element<Is, Tuple>::type>::create(std::get<Is>(t))))...};
}
} // namespace detail
#endif // DOXYGEN_SHOULD_SKIP_THIS

template <class... Types>
Variant<std::tuple<Types...>>
Variant<std::tuple<Types...>>::create(const std::tuple<Types...>& data)
{
  // create a vector containing all tuple values as variants
  std::vector<Glib::VariantBase> variants;
  detail::expand_tuple(variants, data, detail::index_sequence_for<Types...>{});

  using var_ptr = GVariant*;
  var_ptr* const var_array = new var_ptr[sizeof... (Types)];

  for (std::vector<VariantBase>::size_type i = 0; i < variants.size(); i++)
    var_array[i] = const_cast<GVariant*>(variants[i].gobj());

  Variant<std::tuple<Types...>> result = Variant<std::tuple<Types...>>(
          g_variant_new_tuple(var_array, variants.size()));

  return result;
}

template <class... Types>
template <class T>
T Variant<std::tuple<Types...>>::get_child(gsize index) const
{
  Variant<T> entry;
  VariantContainerBase::get_child(entry, index);
  return entry.get();
}

template <class... Types>
template <class T>
Variant<T> Variant<std::tuple<Types...>>::get_child_variant(gsize index) const
{
  Variant<T> entry;
  VariantContainerBase::get_child(entry, index);
  return entry;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
namespace detail
{
// swallows any argument
template <class T>
constexpr int any_arg(T&& /* arg */)
{
  return 0;
}

template <class Tuple, std::size_t... Is>
void assign_tuple(std::vector<VariantBase> &variants, Tuple & t, detail::index_sequence<Is...>)
{
  int i = 0;
  using swallow = int[]; // ensures left to right order
  (void)swallow {(any_arg(std::get<Is>(t) = VariantBase::cast_dynamic<Variant<typename std::tuple_element<Is, Tuple>::type > >(variants[i++]).get()))...};
}
} // namespace detail
#endif // DOXYGEN_SHOULD_SKIP_THIS

template <class... Types>
std::tuple<Types...> Variant<std::tuple<Types...>>::get() const
{
  std::tuple<Types...> data;
  int i = 0;

  std::vector<VariantBase> variants;
  using swallow = int[]; // ensures left to right order
  auto expander = [&variants, &i](const VariantBase &variant) -> int
  {
    variants.push_back(variant);
    return i++;
  };
  (void)swallow{(expander(get_child_variant<Types>(i)))...};
  detail::assign_tuple(variants, data, detail::index_sequence_for<Types...>{});

  return data;
}

template< class... Types>
VariantIter Variant<std::tuple<Types...>>::get_iter() const
{
  const auto type = variant_type();
  return VariantContainerBase::get_iter(type);
}

}

#endif /* NUC_SETTINGS_VARIANT_TUPLE_H */


// Local Variables:
// mode: c++
// End:
