#include "RecoLocalMuon/GEMSegment/plugins/GE0SegAlgoDLTools.h"

namespace ge0 {

// NOTE template<typename T> ??
MinMaxScaler::MinMaxScaler(float x_in_min,
                           float x_in_max,
                           float x_out_min,
                           float x_out_max)
    : x_in_min_(x_in_min),
      x_in_max_(x_in_max),
      x_in_scale_(x_in_max - x_in_min),
      x_out_min_(x_out_min),
      x_out_max_(x_out_max),
      x_out_scale_(x_out_max - x_out_min) {}

MinMaxScaler::~MinMaxScaler() {}

float MinMaxScaler::transform(const float x) const {
  const float x_std = (x - x_in_min_) / x_in_scale_;
  const float x_scaled = x_std * x_out_scale_ + x_out_min_;
  return x_scaled;
}

SuperChamberScaler::SuperChamberScaler(const GEMSuperChamber* superchamber, const float bx_scale)
      : kBXScale_(bx_scale) {

  float x_min = std::numeric_limits<float>::max(); 
  float x_max = std::numeric_limits<float>::min(); 

  float y_min = std::numeric_limits<float>::max(); 
  float y_max = std::numeric_limits<float>::min(); 

  float z_min = std::numeric_limits<float>::max(); 
  float z_max = std::numeric_limits<float>::min(); 

  for (const GEMChamber* chamber : superchamber->chambers()) {
    for (const GEMEtaPartition* eta_partition : chamber->etaPartitions()) {
      for (const int strip : {0, eta_partition->nstrips()}) {

        const LocalPoint&& position = superchamber->toLocal(
            eta_partition->toGlobal(eta_partition->centreOfStrip(strip)));

        x_min = std::min(x_min, position.x());
        x_max = std::max(x_max, position.x());

        y_min = std::min(y_min, position.y());
        y_max = std::max(y_max, position.y());

        z_min = std::min(z_min, position.z());
        z_max = std::max(z_max, position.z());
      }
    }
  }

  valid_ = (x_min < x_max) and (y_min < y_max) and (z_min < z_max);

  x_scaler_ = std::make_unique<MinMaxScaler>(x_min, x_max, -1.0f, 1.0f);
  y_scaler_ = std::make_unique<MinMaxScaler>(y_min, y_max, -1.0f, 1.0f);
  z_scaler_ = std::make_unique<MinMaxScaler>(z_min, z_max, -1.0f, 1.0f);
}

SuperChamberScaler::~SuperChamberScaler() {}

float SuperChamberScaler::transformX(float x) const { return x_scaler_->transform(x); }
float SuperChamberScaler::transformY(float y) const { return y_scaler_->transform(y); }
float SuperChamberScaler::transformZ(float z) const { return z_scaler_->transform(z); }
float SuperChamberScaler::transformBX(float bx) const { return bx / kBXScale_; }
bool SuperChamberScaler::isValid() const { return valid_; }

std::vector<uint32_t> computeTopKIndices(const std::vector<float>& values, const int k) {
  std::vector<uint32_t> indices(values.size());
  std::iota(indices.begin(), indices.end(), 0);

  std::nth_element(indices.begin(), indices.end() - k, indices.end(),
                   [&values](const uint32_t& lhs, const uint32_t& rhs) -> bool {
                       return values.at(lhs) < values.at(rhs);});

  const std::vector<uint32_t> top_k_indices(indices.end() - k, indices.end());
  return top_k_indices;
}

}
