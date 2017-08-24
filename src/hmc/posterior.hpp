//
// Created by Lars Gebraad on 18-8-17.
//

#ifndef HMC_LINEAR_SYSTEM_POSTERIOR_HPP
#define HMC_LINEAR_SYSTEM_POSTERIOR_HPP

#include <AlgebraLib/src/algebra_lib/algebra_lib.hpp>
#include "prior.hpp"
#include "data.hpp"

using namespace algebra_lib;

namespace hmc {
    class posterior {
        double misfit(sparse_vector &parameters, prior &prior, data &data);

        sparse_vector gradient_misfit(sparse_vector &parameters, prior &prior, data &data);
    };
}

#endif //HMC_LINEAR_SYSTEM_POSTERIOR_HPP