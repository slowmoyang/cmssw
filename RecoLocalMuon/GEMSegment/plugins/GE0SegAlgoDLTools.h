#ifndef RecoLocalMuon_GEMSegment_MinMaxScaler_h
#define RecoLocalMuon_GEMSegment_MinMaxScaler_h

#include "Geometry/GEMGeometry/interface/GEMSuperChamber.h"
#include "Geometry/GEMGeometry/interface/GEMChamber.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartition.h"

#include <limits>
#include <numeric>

namespace ge0 {

// NOTE template<typename T> ??
class MinMaxScaler {
 public:
  MinMaxScaler(float x_in_min,
               float x_in_max,
               float x_out_min=-1.0f,
               float x_out_max=1.0f);
  ~MinMaxScaler();

  float transform(const float x) const;

   const float x_in_min_;
   const float x_in_max_;
   const float x_in_scale_;
   const float x_out_min_;
   const float x_out_max_;
   const float x_out_scale_;
};

class SuperChamberScaler {
 public:
  SuperChamberScaler(const GEMSuperChamber* superchamber, const float bx_scale);
  ~SuperChamberScaler();

  float transformX(float x) const;
  float transformY(float y) const;
  float transformZ(float z) const;
  float transformBX(float bx) const;
  bool isValid() const;

  std::unique_ptr<MinMaxScaler> x_scaler_;
  std::unique_ptr<MinMaxScaler> y_scaler_;
  std::unique_ptr<MinMaxScaler> z_scaler_;
  const float kBXScale_;

 private:
  bool valid_; 
};

std::vector<uint32_t> computeTopKIndices(const std::vector<float>& values, const int k);
}

#endif // RecoLocalMuon_GEMSegment_MinMaxScaler_h
