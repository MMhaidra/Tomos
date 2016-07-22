#pragma once

#include <experimental/optional>
using std::experimental::optional;

#include "../common.hpp"
#include "../geometry.hpp"
#include "../image.hpp"
#include "../math.hpp"
#include "../sinogram.hpp"
#include "../volume.hpp"

namespace tomo {

template <dimension D, typename T, class Geometry, class Projector>
image<D, T> art(const volume<D>& v, const Geometry& g,
                const sinogram<D, T, Geometry, Projector>& p, double beta = 0.5,
                int iterations = 10,
                optional<image<D, T>> initial = optional<image<D, T>>()) {
    image<D, T> f(v);
    if (initial)
        f = initial.value();
    Projector proj(v);

    // compute $w_i \cdot w_i$
    std::vector<T> w_norms(g.lines());
    int line_number = 0;
    for (auto line : g) {
        proj.reset(line);
        for (auto elem : proj) {
            w_norms[line_number] += elem.value * elem.value;
        }
        ++line_number;
    }

    for (int k = 0; k < iterations; ++k) {
        int row = 0;
        for (auto line : g) {
            proj.reset(line);
            T alpha = 0.0;
            for (auto elem : proj)
                alpha += f[elem.index] * elem.value;

            auto factor = beta * ((p[row] - alpha) / w_norms[row]);
            for (auto elem : proj)
                f[elem.index] += factor * elem.value;

            ++row;
        }
    }

    return f;
}

} // namespace tomo
