#include "DaphneV3ControllerModule.hpp"
#include "logging/Logging.hpp"
#include "daphnemodules/daphne_control_high.pb.h"

#include "appmodel/DaphneConf.hpp"
#include "appmodel/DaphneV2BoardConf.hpp"
#include "appmodel/DaphneV2Channel.hpp"
#include "appmodel/DaphneV2AFE.hpp"
#include "appmodel/DaphneV2ADC.hpp"
#include "appmodel/DaphneV2PGA.hpp"
#include "appmodel/DaphneV2LNA.hpp"

#include "daphnemodules/opmon/DaphneControllerModule.pb.h"

#include <fmt/format.h>
#include <regex>

#include <zmq.hpp>

namespace dunedaq::daphnemodules {

DaphneV3ControllerModule::DaphneV3ControllerModule(const std::string& name)
  : appfwk::DAQModule(name)
{
  register_command("conf",  &DaphneV3ControllerModule::do_conf);
  register_command("start", &DaphneV3ControllerModule::do_start);
  register_command("scrap", &DaphneV3ControllerModule::do_scrap);
}

void DaphneV3ControllerModule::init(std::shared_ptr<appfwk::ConfigurationManager> cfg)
{
  auto mdal = cfg->get_dal<conf_t>(get_name());
  if (!mdal) {
    throw ConfigurationFailed(ERS_HERE, get_name());
  }
  m_module_config = mdal;
}


void DaphneV3ControllerModule::do_conf(const CommandData_t&)
{

  const std::lock_guard<std::mutex> lock(m_mutex);
  
  TLOG() << get_name() << " starting configuring";
  auto start_time = std::chrono::high_resolution_clock::now();

  
  using namespace daphne;

  auto board_conf = m_module_config->get_board_conf();
  auto general_conf = m_module_config->get_daphne_conf();

  create_interface( board_conf->get_address(),
		    general_conf->get_timeout() );

  // validation to be taken from the previous version
  // this should include the slot check
  validate_configuration(*board_conf);

  configure_analog_chain(true);

  auto end_time = std::chrono::high_resolution_clock::now();
  
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  TLOG() << get_name() << ": board configured in " << duration.count() << " microseconds";
  
}

void DaphneV3ControllerModule::do_start(const CommandData_t& )  { /* nothing yet */ }
void DaphneV3ControllerModule::do_scrap(const CommandData_t&)  {

  m_scrap_called.store(true);
  const std::lock_guard<std::mutex> lock(m_mutex);
  
  TLOG() << get_name() << " starting scrap";
  auto start_time = std::chrono::high_resolution_clock::now();

  using namespace daphne;

  configure_analog_chain(false);

  m_iface = nullptr;

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  TLOG() << get_name() << ": scrapped in " << duration.count() << " microseconds";
}

void DaphneV3ControllerModule::configure_analog_chain(bool initial_config) {

  auto general_conf = m_module_config->get_daphne_conf();
  auto board_conf = initial_config ? m_module_config->get_board_conf() :
    general_conf->get_default_v3_settings();

    // Step 1: Build the ConfigureRequest
  ConfigureRequest req;
  req.set_daphne_address(board_conf->get_address());
  req.set_slot(board_conf->get_slot_id());
  req.set_timeout_ms(general_conf->get_timeout_ms());
  req.set_biasctrl(board_conf->get_bias_ctrl());
  req.set_self_trigger_threshold(board_conf->get_self_trigger_threshold());
  req.set_self_trigger_xcorr(board_conf->get_self_trigger_xcorr());
  req.set_tp_conf(board_conf->get_tp_conf());
  req.set_compensator(board_conf->get_compensator());
  req.set_inverters(board_conf->get_inverter());

  for ( ChannelId ch = 0; ch < s_max_channels; ++ch ) {

    const auto & channel_conf = board_conf->get_channel(ch);
    // get_channel returns the running value if the bool argument is true,
    // and the default values when the bool argument is false
    // hence, this loop does both the job of enabling and disebling

    auto* channel = req.add_channels();
    channel->set_id(ch);
    channel->set_trim(channel_conf.get_trim());
    channel->set_offset(channel_conf.get_offset());
    channel->set_gain(channel_conf.get_gain());
    
  } // loop over channels


  for ( AFEId id = 0; id < s_max_afes; ++id ) {

    const auto & afe_conf = board_conf->get_afe(id);

    auto* afe = req.add_afes();
    afe->set_id(id);
    afe->set_attenuators(afe_conf.get_attenuator());
    afe->set_v_bias(afe_conf.get_v_bias());

    auto* adc = afe_conf.get_adc();
    afe->mutable_adc()->set_resolution(adc->get_low_resolution());
    afe->mutable_adc()->set_output_format(adc->get_output_offset_binary());
    afe->mutable_adc()->set_sb_first(adc->get_MSB_first());

    auto* pga = afe_conf.get_pga();
    afe->mutable_pga()->set_lpf_cut_frequency(pga->get_lpf_cut_frequency());
    afe->mutable_pga()->set_integrator_disable(pga->get_integrator_disable());
    afe->mutable_pga()->set_gain(pga->get_gain());

    auto* lna = afe_conf.get_lna();
    afe->mutable_lna()->set_clamp(lna->get_clamp());
    afe->mutable_lna()->set_gain(lna->get_gain());
    afe->mutable_lna()->set_integrator_disable(lna->get_integrator_disable());
        
  }  // loop over AFE
  
  TLOG() << get_name() << ": Configuration message ready to send";

  auto response = m_iface.load()->send<ConfigureResponse>( req.SerializeAsString(),
							   MT2_CONFIGURE_FE_REQ,
							   MT2_CONFIGURE_FE_RESP );
  
  if ( ! response.success() ) {
    throw UnsuccessfulConfiguration(ERS_HERE, get_name(), response.message());
  }

  TLOG() << "Success message: " << response.message();
  
  return;
}

  void DaphneV3ControllerModule::create_interface( const std::string & address,
						   std::chrono::milliseconds timeout )  {
    
    m_iface = make_unique<DaphneV3Interface>( address, get_name(), timeout);
    
  }

  void DaphneV3ControllerModule::validate_configuration(const appmodel::DaphneV2BoardConf & c) const {
    
    const auto & channel_confs = c.get_active_channels();

  for ( const auto & ch : channel_confs ) {
    auto id = ch->get_channel_id();
    
    //CH OFFSET maximum is 2700 if GAIN is 1, 1500 if GAIN is 2
    auto gain = ch->get_gain();
    if ( gain != 1 && gain != 2 ) {
      throw InvalidChannelConfiguration(ERS_HERE,
                                        id, ch->get_trim(), ch->get_offset(), gain);
    }
    auto offset = ch -> get_offset();
    if ( gain == 1 ) {
      if ( offset > 2700 ) 
        throw InvalidChannelConfiguration(ERS_HERE, id, ch->get_trim(), offset, gain);
    } else if ( gain == 2 ) {
      if ( offset > 1500 ) 
        throw InvalidChannelConfiguration(ERS_HERE, id, ch->get_trim(), offset, gain);
    }
  } // loop over channels

  auto size = c.get_full_stream_channels().size();
  if (size>16) {
    // we can only stream 16 channels at most
    throw TooManyChannels( ERS_HERE, size );
  }

  }

  void
  DaphneV3ControllerModule::generate_opmon_data() {

    if ( ! m_iface.load() ) return ;

    if ( m_scrap_called.load() ) return;

    std::unique_lock<std::mutex> lock(m_mutex);

    ReadTriggerCountersRequest req;
    auto response = m_iface.load()->send<ReadTriggerCountersResponse>( req.SerializeAsString(),
								       MT2_READ_TRIGGER_COUNTERS_REQ,
								       MT2_READ_TRIGGER_COUNTERS_RESP );

    if ( ! response.success() ) {
      ers::warning( TriggerMonitoringFailed(ERS_HERE, get_name(), response.message() ) );
      return;
    }

    lock.unlock();
    
    const auto snapshots = response.snapshots();

    static uint32_t def_threshold = 0x3ff; 
    
    for ( const auto & c : snapshots ) {

      // we only publish channels info when threshold is not default or the counters are not zero
      if ( c.threshold() == def_threshold
	   && c.record_count() == 0
	   && c.busy_count() == 0
	   && c.full_count() == 0 ) continue;
      
      opmon::TempTriggerSnapshotInfo info;
      info.set_threshold( c.threshold() );
      info.set_record_count( c.record_count() );
      info.set_busy_count( c.busy_count() );
      info.set_full_count( c.full_count() );

      publish( std::move(info), {{"channel", fmt::format("{}", c.channel() ) }} );
    }
    
  }

  
} // namespace dunedaq::daphnemodules

DEFINE_DUNE_DAQ_MODULE(dunedaq::daphnemodules::DaphneV3ControllerModule)
