#pragma once

#include <limits>

#include "../geometry.hpp"
#include "../math.hpp"
#include "../volume.hpp"
#include "trajectory.hpp"

namespace tomo {
namespace geometry {

/**
 * A circular cone-beam geometry
 *
 * \tparam D the dimension of the volume.
 * \tparam T the scalar type to use
 */
template <typename T>
class cone_beam : public trajectory<3_D, T> {
  public:
    /** Construct the geometry with a given number of lines. */
    cone_beam(volume<3_D> volume, int steps, T detector_spacing = (T)1.0,
              math::vec<2_D, int> detector_size = math::vec<2_D, int>{1},
              T relative_source_distance = (T)1.0,
              T relative_detector_distance = (T)1.0)
        : trajectory<3_D, T>(volume, steps, detector_spacing, detector_size),
          relative_source_distance_(relative_source_distance),
          relative_detector_distance_(relative_detector_distance) {}

    math::vec<3_D, T> source_location(int step) const override final {
        return transform_location_(
            image_center_() - (relative_source_distance_ * this->volume_[0] *
                               math::standard_basis<3_D, T>(0)),
            step);
    }

    math::vec<3_D, T> detector_location(int step) const override final {
        return transform_location_(
            image_center_() + (relative_detector_distance_ * this->volume_[0] *
                               math::standard_basis<3_D, T>(0)),
            step);
    }

    std::array<math::vec<3_D, T>, 2>
    detector_tilt(int step) const override final {
        return {apply_rotation_(math::standard_basis<3_D, T>(1), step),
                apply_rotation_(math::standard_basis<3_D, T>(2), step)};
    }

  private:
    T relative_source_distance_;
    T relative_detector_distance_;

    inline math::vec<3_D, T> transform_location_(math::vec<3_D, T> location,
                                                 int step) const {
        return apply_rotation_(location - image_center_(), step) +
               image_center_();
    }

    inline math::vec<3_D, T> apply_rotation_(math::vec<3_D, T> location,
                                             int step) const {
        static auto axis = math::standard_basis<3_D, T>(1);

        T angle_step = (T)2.0 * math::pi<T> / this->steps_;

        return math::rotate(location, axis, angle_step * step);
    }

    inline math::vec<3_D, T> image_center_() const {
        math::vec<3_D, T> image_center = {this->volume_.x() * (T)0.5,
                                          this->volume_.y() * (T)0.5,
                                          this->volume_.z() * (T)0.5};

        return image_center;
    }
};

} // namespace geometry
} // namespace tomo
