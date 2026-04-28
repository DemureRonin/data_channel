#include "i_compressor.h"

 class MantissaCompressor : public  ICompressor {
 public:
     MantissaCompressor() = default;

     DataPacket Compress(DataPacket data) override {
          return {};
     }
 };