/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef HSM_DETAIL_BACK_HPP_INCLUDED
#define HSM_DETAIL_BACK_HPP_INCLUDED
#include <hsm/hsm_fwd.hpp>
#include <kvasir/mpl/algorithm/stable_sort.hpp>
#include <kvasir/mpl/algorithm/find_if.hpp>
#include <kvasir/mpl/sequence/size.hpp>

namespace hsm
{
namespace back
{
namespace detail
{
template <typename C>
struct count
{
    constexpr static size_t value = 1;
};

template <>
struct count<hsm::no_action>
{
    constexpr static size_t value = 0;
};

template <>
struct count<hsm::no_cond>
{
    constexpr static size_t value = 0;
};

template <typename C, typename T>
struct get_state_impl
{
    using type = T;
};

template <typename C>
struct get_state_impl<C, hsm::current_state>
{
    using type = C;
};

template <typename C>
struct get_state_impl<C, hsm::internal_transition>
{
    using type = C;
};

template <typename C>
struct get_state_impl<C, hsm::initial_state>
{
    using type = C;
};

template <typename C, typename T>
struct get_state_impl<C, hsm::history_state<T>>
{
    using type = typename get_state_impl<C, T>::type;
};

template <typename C, typename T>
struct get_state_impl<C, hsm::deep_history_state<T>>
{
    using type = typename get_state_impl<C, T>::type;
};

template <typename C, typename T>
struct get_state_impl<C, hsm::final_state<T>>
{
    using type = typename get_state_impl<C, T>::type;
};

struct is_action
{
    template <typename T>
    struct f_impl : kvasir::mpl::bool_<false>
    {
    };
    template <typename A>
    struct f_impl<hsm::action_node<A>> : kvasir::mpl::bool_<true>
    {
    };
    template <typename A>
    struct f_impl<hsm::entry_action<A>> : kvasir::mpl::bool_<true>
    {
    };
    template <typename A>
    struct f_impl<hsm::exit_action<A>> : kvasir::mpl::bool_<true>
    {
    };
    template <typename T>
    using f = f_impl<T>;
};

struct is_condition
{
    template <typename T>
    struct f_impl : kvasir::mpl::bool_<false>
    {
    };
    template <typename A>
    struct f_impl<condition_node<A>> : kvasir::mpl::bool_<true>

    {
    };
    template <typename T>
    using f = f_impl<T>;
};

struct function_type
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };
    template <typename A>
    struct f_impl<hsm::condition_node<A>>
    {
        using type = A;
    };

    template <typename A>
    struct f_impl<hsm::action_node<A>>
    {
        using type = A;
    };
    template <typename A>
    struct f_impl<hsm::entry_action<A>>
    {
        using type = A;
    };
    template <typename A>
    struct f_impl<hsm::exit_action<A>>
    {
        using type = A;
    };
    template <typename T>
    using f = typename f_impl<T>::type;
};

template <int Id>
struct is_state
{
    template <typename T>
    struct f_impl : kvasir::mpl::bool_<false>
    {
    };
    template <typename N, uint32_t F, size_t S, size_t P, size_t Entry, size_t Exit, typename... Ts>
    struct f_impl<tiny_tuple::detail::item<N, hsm::back::state<F, Id, S, P, Entry, Exit, Ts...>>> : kvasir::mpl::bool_<true>
    {
    };
    template <typename T>
    using f = f_impl<T>;
};

struct is_any_state
{
    template <typename T>
    struct f_impl : kvasir::mpl::bool_<false>
    {
    };
    template <typename N, uint32_t F, size_t Id, size_t S, size_t P, size_t Entry, size_t Exit, typename... Ts>
    struct f_impl<tiny_tuple::detail::item<N, hsm::back::state<F, Id, S, P, Entry, Exit, Ts...>>> : kvasir::mpl::bool_<true>
    {
    };
    template <typename T>
    using f = f_impl<T>;
};

struct sort_transition
{
    constexpr static transition_flags tm       = transition_flags::transition_type_mask;
    constexpr static transition_flags internal = transition_flags::internal;
    constexpr static transition_flags normal   = transition_flags::normal;
    template <typename T1, typename T2>
    using f = kvasir::mpl::bool_<(T1::flags & tm) < (T2::flags & tm) ||
                                 (((T1::flags & tm) == (T2::flags & tm) && ((((T2::flags & tm) == normal) && T1::event > T2::event) ||
                                                                            ((((T2::flags & tm) == internal) && T1::event > T2::event)))))>;
};

struct by_state_id
{
    template <typename T1, typename T2>
    using f = kvasir::mpl::bool_<(T1::value::id < T2::value::id)>;
};

struct extract_transition_count
{
    template <typename T>
    using f = kvasir::mpl::uint_<T::transition_count>;
};

struct normal_transition
{
    template <typename T1>
    using f = kvasir::mpl::bool_<(T1::flags & transition_flags::transition_type_mask) >= transition_flags::internal>;
};

template <typename C = kvasir::mpl::listify>
struct unpack_transitions
{
    template <typename... T>
    using f = typename kvasir::mpl::detail::unpack_impl<C, typename T::transitions...>::type;
};

template <typename C, typename T>
struct flatten_state_machine
{
    using type = kvasir::mpl::list<T>;
};
template <typename C, typename K, typename... Ts>
struct flatten_state_machine<C, hsm::state<K, Ts...>>
{
    using type = typename kvasir::mpl::join<C>::template f<typename flatten_state_machine<C, Ts>::type...>::type;
};

template <typename T>
struct flatten_transition
{
    using type = kvasir::mpl::list<T>;
};
template <typename K, typename... Ts>
struct flatten_transition<hsm::state<K, Ts...>>
{
    using type = typename kvasir::mpl::join<kvasir::mpl::listify>::template f<typename detail::flatten_transition<Ts>::type...>;
};
template <typename TT, typename S, typename E, typename C, typename A, typename D>
struct flatten_transition<hsm::transition<TT, S, E, C, A, D>>
{
    using type = kvasir::mpl::list<S, E, C, A, D>;
};

}  // namespace detail
}  // namespace back
}  // namespace hsm

#endif
