#ifndef HSM_DETAIL_BACK_HPP_INCLUDED
#define HSM_DETAIL_BACK_HPP_INCLUDED
#include <hsm/hsm_fwd.hpp>

namespace hsm
{
namespace back
{
namespace detail
{
template <typename T>
struct get_action_impl
{
    using type = kvasir::mpl::uint_<std::numeric_limits<long long unsigned int>::max()>;
};

template <typename A, size_t Id>
struct get_action_impl<action_node<A, Id>>
{
    using type = kvasir::mpl::uint_<Id>;
};

template <typename T>
struct get_condition_impl
{
    using type = kvasir::mpl::uint_<std::numeric_limits<long long unsigned int>::max()>;
};

template <typename C, size_t Id>
struct get_action_impl<condition_node<C, Id>>
{
    using type = kvasir::mpl::uint_<Id>;
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

template <typename T, typename Conditions, typename Actions>
constexpr void initialize_ca_array(T&&, Conditions&, Actions&)
{
}

template <typename A, size_t I, typename Actions>
constexpr void set_action(action_node<A, I>&& node, Actions& actions)
{
    actions[I] = std::move(node.action);
}
template <typename C, size_t I, typename Conditions>
constexpr void set_condition(condition_node<C, I>&& node, Conditions& conds)
{
    conds[I] = std::move(node.cond);
}
template <typename Conditions>
constexpr void set_condition(no_cond&&, Conditions&)
{
}
template <typename Actions>
constexpr void set_action(no_action&&, Actions&)
{
}

template <typename A, size_t I, typename Conditions, typename Actions>
constexpr void initialize_ca_array(exit_action<A, I>&& exit, Conditions&, Actions& actions)
{
    actions[I] = std::move(exit.action);
}
template <typename A, size_t I, typename Conditions, typename Actions>
constexpr void initialize_ca_array(entry_action<A, I>&& entry, Conditions&, Actions& actions)
{
    actions[I] = std::move(entry.action);
}
template <typename TT, typename S, typename E, typename C, typename A, typename D, typename Conditions, typename Actions>
constexpr void initialize_ca_array(hsm::transition<TT, S, E, C, A, D>&& sm, Conditions& conds, Actions& actions)
{
    set_action(std::move(sm.action), actions);
    set_condition(std::move(sm.cond), conds);
}
template <typename K, typename... Cs, typename Conditions, typename Actions>
constexpr void initialize_ca_array(hsm::state<K, Cs...>&& sm, Conditions& conds, Actions& actions)
{
    tiny_tuple::foreach (std::move(sm.data), [&conds, &actions](auto&& element) {
        initialize_ca_array(std::forward<decltype(element)>(element), conds, actions);
    });
}

template <int Id>
struct is_state
{
    template <typename T>
    struct f_impl : kvasir::mpl::bool_<false>
    {
    };
    template <typename N, uint32_t F, size_t S, size_t P, typename... Ts>
    struct f_impl<tiny_tuple::detail::item<N, hsm::back::state<F, Id, S, P, Ts...>>> : kvasir::mpl::bool_<true>
    {
    };
    template <typename T>
    using f = f_impl<T>;
};

template <typename D, typename S>
constexpr D convert_max(S s)
{
    if (s == std::numeric_limits<S>::max()) return std::numeric_limits<D>::max();
    return static_cast<D>(s);
}

template <int TO, typename TT, size_t E, size_t D, size_t C, size_t A, typename Transitions>
constexpr int apply_transition(hsm::back::transition<TT, E, D, C, A>, Transitions& trans)
{
    using tte = typename Transitions::value_type;
    trans[TO] = tte{typename tte::event_id(E), typename tte::state_id(D), convert_max<typename tte::condition_id>(C),
                    convert_max<typename tte::action_id>(A), 0};
    return 0;
}

template <int TO, uint32_t Flags, size_t Id, size_t StateCount, size_t Parent, typename... Ts, int... Is, typename Transitions>
constexpr void apply_transitions(hsm::back::state<Flags, Id, StateCount, Parent, Ts...>, std::integer_sequence<int, Is...>,
                                 Transitions& trans)
{
    auto f = {apply_transition<TO + Is>(Ts{}, trans)...};
    (void)f;
}

template <int I, int TO, uint32_t Flags, size_t Id, size_t StateCount, size_t Parent, typename... Ts, typename Transitions, typename States>
constexpr auto apply_state(hsm::back::state<Flags, Id, StateCount, Parent, Ts...> sm, Transitions& transitions, States& states)
{
    using state_table_entry = typename States::value_type;
    states[I]               = state_table_entry{static_cast<typename state_table_entry::transition_table_offset_type>(TO),
                                  static_cast<typename state_table_entry::state_id>(Parent), static_cast<uint16_t>(sizeof...(Ts)), Flags};
    apply_transitions<TO>(sm, std::make_integer_sequence<int, sizeof...(Ts)>(), transitions);
    return kvasir::mpl::uint_<TO + sizeof...(Ts)>();
}

template <int I, int TO, typename SM, typename Transitions, typename State>
constexpr void find_and_apply_state(SM&& sm, Transitions& transitions, std::array<State, I>& states)
{
}

template <int I, int TO, typename SM, typename Transitions, typename State, size_t SC>
constexpr void find_and_apply_state(SM&& sm, Transitions& transitions, std::array<State, SC>& states)
{
    using state_found = typename kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::find_if<is_state<I>, kvasir::mpl::front<>>>, SM>::value;
    auto transition_offset = apply_state<I, TO>(state_found{}, transitions, states);
    find_and_apply_state<I + 1, decltype(transition_offset)::value>(std::forward<SM>(sm), transitions, states);
}

}  // namespace detail
}  // namespace back
}  // namespace hsm

#endif