#include <iostream>
#include <type_traits>
#include <hsm/hsm.hpp>

int main()
{
    namespace hb = hsm::back;
    using ti     = hb::transition<static_cast<uint32_t>(hb::transition_flags::initial), 0, 0, 0, 0>;
    using tc     = hb::transition<static_cast<uint32_t>(hb::transition_flags::completion), 0, 0, 0, 0>;
    using th     = hb::transition<static_cast<uint32_t>(hb::transition_flags::history), 0, 0, 0, 0>;
    using tint   = hb::transition<static_cast<uint32_t>(hb::transition_flags::internal), 0, 0, 0, 0>;
    using tnorm  = hb::transition<static_cast<uint32_t>(hb::transition_flags::normal), 0, 0, 0, 0>;
    {
        using input              = kvasir::mpl::list<tnorm, th, ti, tint, tc>;
        using expected           = kvasir::mpl::list<ti, th, tc, tint, tnorm>;
        using sorted_transitions = kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::stable_sort<hb::detail::sort_transition>>, input>;
        using num_normal_transition =
            kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::find_if<hb::detail::normal_transition, kvasir::mpl::size<>>>,
                              sorted_transitions>;
        auto const special_tr = static_cast<uint8_t>(size_t(5) - num_normal_transition::value);
        static_assert(std::is_same<sorted_transitions, expected>::value);
        static_assert(special_tr == 3);
    }

    {
        using input              = kvasir::mpl::list<tnorm>;
        using expected           = kvasir::mpl::list<tnorm>;
        using sorted_transitions = kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::stable_sort<hb::detail::sort_transition>>, input>;
        using num_normal_transition =
            kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::find_if<hb::detail::normal_transition, kvasir::mpl::size<>>>,
                              sorted_transitions>;
        auto const special_tr = static_cast<uint8_t>(size_t(1) - num_normal_transition::value);
        static_assert(std::is_same<sorted_transitions, expected>::value);
        static_assert(special_tr == 0);
    }

    {
        using input              = kvasir::mpl::list<tint>;
        using expected           = kvasir::mpl::list<tint>;
        using sorted_transitions = kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::stable_sort<hb::detail::sort_transition>>, input>;
        using num_normal_transition =
            kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::find_if<hb::detail::normal_transition, kvasir::mpl::size<>>>,
                              sorted_transitions>;
        auto const special_tr = static_cast<uint8_t>(size_t(1) - num_normal_transition::value);
        static_assert(std::is_same<sorted_transitions, expected>::value);
        static_assert(special_tr == 0);
    }

    {
        using type = hb::state<49u, 2ul, 1ul, 1ul, 4ul, 5ul, hsm::back::transition<0u, 0ul, 3ul, 0ul, 0ul>,
                               hsm::back::transition<4 | 32, 2ul, 5ul, 0ul, 9ul>>;
        std::array<hsm::detail::tt_entry<unsigned char, unsigned char, unsigned char, unsigned char>, 2ul> trs;
        std::array<hsm::detail::state_entry<unsigned char, unsigned char, unsigned char>, 1ul>             sts;
        hsm::back::detail::apply_state<0, 0>(type{}, trs, sts);
        if (sts[0].transition_count == 2 && sts[0].special_transition_count == 1) { std::cout << "success\n"; }
        else
        {
            std::cout << "failed \n";
            return 1;
        }
    }
}
