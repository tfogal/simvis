#include "Controller/Controller.h"
#include "IO/RAWConverter.h"

// RAII for adding/removing a debug channel from the controller.
class AddADebugOut {
  public:
    AddADebugOut(tuvok::MasterController& ctl) : ctlr(ctl), debugOut(NULL) {
      this->debugOut = new ConsoleOut();
      this->debugOut->SetOutput(true, true, true, false);
      this->ctlr.AddDebugOut(this->debugOut);
    }
    ~AddADebugOut() {
      this->ctlr.RemoveDebugOut(this->debugOut);
      // controller will clean up the debugOut automagically; we don't need to
      // delete it.
    }
  private:
    tuvok::MasterController& ctlr;
    ConsoleOut* debugOut;
};

// assumes floating point fata
void uvf_convert(const char* input, const char* uvf, const size_t dimensions[3])
{
  const std::string temp_dir = ".";
  const uint64_t skip_bytes = 0;
  const uint64_t component_size = 32;
  const uint64_t component_count = 1;
  const uint64_t timesteps = 1;
  const bool convert_endianness = false;
  const bool is_signed = true;
  const bool is_float = true;
  UINT64VECTOR3 dims = UINT64VECTOR3(dimensions[0], dimensions[1],
                                     dimensions[2]);
  const FLOATVECTOR3 aspect = FLOATVECTOR3(1.0, 1.0, 1.0);
  const uint64_t brick_size = 256;
  const uint64_t overlap = 4;

  AddADebugOut dbgout(tuvok::Controller::Instance());
  if(RAWConverter::ConvertRAWDataset(std::string(input), std::string(uvf),
                                     temp_dir,
                                     skip_bytes, component_size,
                                     component_count, timesteps,
                                     convert_endianness, is_signed, is_float,
                                     dims, aspect,
                                     std::string("In-progress simulation"),
                                     std::string("AGENT"),
                                     brick_size, overlap)) {
    MESSAGE("Success!");
  } else {
    T_ERROR("Conversion failed!");
  }
}
