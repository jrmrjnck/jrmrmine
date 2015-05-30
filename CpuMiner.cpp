#include "Miner.h"

#include <memory>

class CpuMiner : public Miner
{
public:
   ~CpuMiner() {}

public:
   static MinerPtr createInstance()
   {
      return MinerPtr( new CpuMiner );
   }
};

MinerRegistration<CpuMiner> registration( "cpu" );
