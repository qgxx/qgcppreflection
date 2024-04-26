#ifndef __TYPE_LIST_H__
#define __TYPE_LIST_H__

template <typename... Args>
struct type_list {
    static constexpr size_t size = sizeof...(Args);
};

namespace detail {

template <typename> struct head;

template <typename T, typename... Remains> 
struct head<type_list<T, Remains...>> {
    using type = T;
};

template <typename> struct tail;

template <typename T, typename... Remains>
struct tail<type_list<T, Remains...>> {
    using type = type_list<Remains...>;
};

template <typename, size_t> struct ithType;

template <typename T, typename... Remains>
struct ithType<type_list<T, Remains...>, 0> {
    using type = T;
};

template <typename T, typename... Remains, size_t N>
struct ithType<type_list<T, Remains...>, N> {
    using type = typename ithType<type_list<Remains...>, N - 1>::type;
};

template <typename, template <typename> typename, size_t> struct count;

template <typename T, typename... Remains, template <typename> typename F>
struct count<type_list<T, Remains...>, F, 0> {
    static constexpr int value = (F<T>::value ? 1 : 0);
};

template <typename T, typename... Remains, template <typename> typename F, size_t N>
struct count<type_list<T, Remains...>, F, N> {
    static constexpr int value = (F<T>::value ? 1 : 0) + count<type_list<Remains...>, F, N - 1>::value;
};

template <typename, template <typename> typename> struct map;

template <typename... Args, template <typename> typename F>
struct map<type_list<Args...>, F> {
    using type = type_list<typename F<Args>::type ...>;
};

template <typename, typename> struct cons;

template <typename T, typename... Args> 
struct cons<T, type_list<Args...>> {
    using type = type_list<T, Args...>;
};

template <typename, typename> struct concat;

template <typename... Args1, typename... Args2> 
struct concat<type_list<Args1...>, type_list<Args2...>> {
    using type = type_list<Args1..., Args2...>;
};

template <typename> struct init;

template <typename T>
struct init<type_list<T>> {
    using type = type_list<>;
};

template <typename T, typename... Remains> 
struct init<type_list<T, Remains...>> {
    using type = typename cons<T, typename init<type_list<Remains...>>::type>::type;
};

template <typename, template <typename> typename> struct filter;

template <template <typename> typename F>
struct filter<type_list<>, F> {
    using type = type_list<>;
};

template <typename T, typename... Remains, template <typename> typename F>
struct filter<type_list<T, Remains...>, F> {
    using type = std::conditional_t<F<T>::value, 
    typename cons<T, typename filter<type_list<Remains...>, F>::type>::type,
    typename filter<type_list<Remains...>, F>::type>;
};

} // namespace detail

template <typename TypeList> 
using head = typename detail::head<TypeList>::type;

template <typename TypeList>
using tail = typename detail::tail<TypeList>::type;

template <typename TypeList, size_t N>
using ithType = typename detail::ithType<TypeList, N>::type;

template <typename TypeList, template <typename> typename F>
constexpr int count = detail::count<TypeList, F, TypeList::size - 1>::value;

template <typename TypeList, template <typename> typename F>
using map = typename detail::map<TypeList, F>::type;

template <typename TypeList, typename T>
using cons = typename detail::cons<T, TypeList>::type;

template <typename TypeList1, typename TypeList2>
using concat = typename detail::concat<TypeList1, TypeList2>::type;

template <typename TypeList>
using init = typename detail::init<TypeList>::type;

template <typename TypeList, template <typename> typename F>
using filter = typename detail::filter<TypeList, F>::type;

#endif // !__TYPE_LIST_H__