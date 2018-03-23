#include <fstream>

#include "tomos/tomos.hpp"
#include "tomos/util/matrix_sums.hpp"
#include "tomos/util/simple_args.hpp"

using namespace tomo;
using namespace tomo::img;

int main(int argc, char** argv) {
    using T = float;
    constexpr dimension D = 2_D;

    auto opts = tomo::options{argc, argv};
    auto method = opts.arg_or("--method", "art");
    auto beta = opts.arg_as_or<T>("-b", (T)1.0);
    auto iterations = opts.arg_as_or<int>("-i", 1);
    auto size = opts.arg_as_or<int>("-s", 128);
    auto output_dir = opts.arg_or("--dir", "/home/jw/data/berlin");
    auto output_base =
        opts.arg_or("--output", output_dir + "/convergence_" + method + "_");

    auto v = tomo::volume<D, T>(size);
    auto g = tomo::geometry::fan_beam<T>(v, size, math::vec<D, T>(2.5),
                                         math::vec<D, int>(size), 4.0, 4.0);
    auto k = tomo::dim::joseph<D, T>(v);
    auto f = tomo::modified_shepp_logan_phantom<T>(v);
    auto p = tomo::forward_projection<D, T>(f, g, k);

    auto err = math::norm(f);
    auto rfout = std::ofstream(output_base + "residuals.csv");
    auto efout = std::ofstream(output_base + "rel_errors.csv");

    auto cb = std::function([&](image<D, T>& xk, int iter) {
        tomo::write_png(xk, output_base + std::to_string(iter));
        if (iter != 0) {
            efout << ", ";
            rfout << ", ";
        }
        auto rk = tomo::forward_projection(xk, g, k);
        rfout << math::norm(p - rk);
        auto derr = math::norm(f - xk);
        efout << derr / err;
        std::cout << iter << " " << derr / err << "\n";
    });

    auto x = tomo::image<D, T>(v);
    if (method == "art") {
        x = tomo::reconstruction::art(v, g, k, p, beta, iterations, cb);
    } else if (method == "sart") {
        x = tomo::reconstruction::sart(v, g, k, p, beta, iterations, cb);
    } else if (method == "sart_cg") {
        x = tomo::reconstruction::sart_cg(v, g, k, p, iterations, beta, cb);
    } else if (method == "sirt") {
        x = tomo::reconstruction::sirt(v, g, k, p, beta, iterations, cb);
    } else if (method == "landweber") {
        x = tomo::reconstruction::landweber(v, g, k, p, beta, iterations, cb);
    } else if (method == "cgls") {
        x = tomo::reconstruction::cgls(v, g, k, p, iterations, cb);
    } else if (method == "cgls2") {
        x = tomo::reconstruction::cgls2(v, g, k, p, iterations, cb);
    } else if (method == "cg") {
        x = tomo::reconstruction::cg(v, g, k, p, iterations, cb);
    } else {
        std::cout << "INVALID METHOD\n";
    }


    efout << "\n";
    rfout << "\n";

    tomo::write_png(x, "test");
}
