#ifndef DQM_SiStripCommissioningSummary_SamplingSummaryFactory_H
#define DQM_SiStripCommissioningSummary_SamplingSummaryFactory_H

#include "DQM/SiStripCommissioningSummary/interface/CommissioningSummaryFactory.h"

class SamplingSummaryFactory : public SummaryPlotFactory<CommissioningAnalysis*> {
protected:
  void extract(Iterator) override;

  void format() override;
};

#endif  // DQM_SiStripCommissioningSummary_SamplingSummaryFactory_H
