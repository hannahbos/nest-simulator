/*
 *  hh_psc_alpha.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HH_PSC_ALPHA_H
#define HH_PSC_ALPHA_H

// Generated includes:
#include "config.h"

#ifdef HAVE_GSL

// External includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>
#include <gsl/gsl_sf_exp.h>

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{
/**
 * Function computing right-hand side of ODE for GSL solver.
 * @note Must be declared here so we can befriend it in class.
 * @note Must have C-linkage for passing to GSL. Internally, it is
 *       a first-class C++ function, but cannot be a member function
 *       because of the C-linkage.
 * @note No point in declaring it inline, since it is called
 *       through a function pointer.
 * @param void* Pointer to model neuron instance.
 */
extern "C" int hh_psc_alpha_dynamics( double, const double*, double*, void* );

/* BeginDocumentation
Name: hh_psc_alpha - Hodgkin Huxley neuron model.

Description:

 hh_psc_alpha is an implementation of a spiking neuron using the Hodkin-Huxley
 formalism.

 (1) Post-syaptic currents
 Incoming spike events induce a post-synaptic change of current modelled
 by an alpha function. The alpha function is normalised such that an event of
 weight 1.0 results in a peak current of 1 pA.


 (2) Spike Detection
 Spike detection is done by a combined threshold-and-local-maximum search: if
 there is a local maximum above a certain threshold of the membrane potential,
 it is considered a spike.

Parameters:

 The following parameters can be set in the status dictionary.

 V_m        double - Membrane potential in mV
 E_L        double - Resting membrane potential in mV.
 g_L        double - Leak conductance in nS.
 C_m        double - Capacity of the membrane in pF.
 tau_ex     double - Rise time of the excitatory synaptic alpha function in ms.
 tau_in     double - Rise time of the inhibitory synaptic alpha function in ms.
 E_Na       double - Sodium reversal potential in mV.
 g_Na       double - Sodium peak conductance in nS.
 E_K        double - Potassium reversal potential in mV.
 g_K        double - Potassium peak conductance in nS.
 Act_m      double - Activation variable m
 Act_h      double - Activation variable h
 Inact_n    double - Inactivation variable n
 I_e        double - Constant external input current in pA.

Problems/Todo:

 better spike detection
 initial wavelet/spike at simulation onset

References:

 Spiking Neuron Models:
 Single Neurons, Populations, Plasticity
 Wulfram Gerstner, Werner Kistler,  Cambridge University Press

 Theoretical Neuroscience:
 Computational and Mathematical Modeling of Neural Systems
 Peter Dayan, L. F. Abbott, MIT Press (parameters taken from here)

 Hodgkin, A. L. and Huxley, A. F.,
 A Quantitative Description of Membrane Current
 and Its Application to Conduction and Excitation in Nerve,
 Journal of Physiology, 117, 500-544 (1952)

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

Authors: Schrader
SeeAlso: hh_cond_exp_traub
*/

class hh_psc_alpha : public Archiving_Node
{

public:
  hh_psc_alpha();
  hh_psc_alpha( const hh_psc_alpha& );
  ~hh_psc_alpha();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  port send_test_event( Node&, rport, synindex, bool );

  void handle( SpikeEvent& );
  void handle( CurrentEvent& );
  void handle( DataLoggingRequest& );

  port handles_test_event( SpikeEvent&, rport );
  port handles_test_event( CurrentEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );

  /**
   * Return membrane potential at time t.
potentials_.connect_logging_device();
   * This function is not thread-safe and should not be used in threaded
   * contexts to access the current membrane potential values.
   * @param Time the current network time
   *
   */
  double get_potential( Time const& ) const;

  /**
   * Define current membrane potential.
   * This function is thread-safe and should be used in threaded
   * contexts to change the current membrane potential value.
   * @param Time     the current network time
   * @param double new value of the mebrane potential
   *
   */
  void set_potential( Time const&, double );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();
  void update( Time const&, const long, const long );

  // END Boilerplate function declarations ----------------------------

  // Friends --------------------------------------------------------

  // make dynamics function quasi-member
  friend int hh_psc_alpha_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friend to access the State_ class/member
  friend class RecordablesMap< hh_psc_alpha >;
  friend class UniversalDataLogger< hh_psc_alpha >;

private:
  // ----------------------------------------------------------------

  //! Independent parameters
  struct Parameters_
  {
    double t_ref_;   //!< refractory time in ms
    double g_Na;     //!< Sodium Conductance in nS
    double g_K;      //!< Potassium Conductance in nS
    double g_L;      //!< Leak Conductance in nS
    double C_m;      //!< Membrane Capacitance in pF
    double E_Na;     //!< Sodium Reversal Potential in mV
    double E_K;      //!< Potassium Reversal Potential in mV
    double E_L;      //!< Leak reversal Potential (aka resting potential) in mV
    double tau_synE; //!< Synaptic Time Constant Excitatory Synapse in ms
    double tau_synI; //!< Synaptic Time Constant for Inhibitory Synapse in ms
    double I_e;      //!< Constant Current in pA

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
  };

public:
  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor and assignment operator required because
   *       of C-style array.
   */
  struct State_
  {

    /**
     * Enumeration identifying elements in state array State_::y_.
     * The state vector must be passed to GSL as a C array. This enum
     * identifies the elements of the vector. It must be public to be
     * accessible from the iteration function.
     */
    enum StateVecElems
    {
      V_M = 0,
      HH_M,   // 1
      HH_H,   // 2
      HH_N,   // 3
      DI_EXC, // 4
      I_EXC,  // 5
      DI_INH, // 6
      I_INH,  // 7
      STATE_VEC_SIZE
    };


    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];
    int r_; //!< number of refractory steps remaining

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  };

  // ----------------------------------------------------------------

private:
  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( hh_psc_alpha& );                  //!<Sets buffer pointers to 0
    Buffers_( const Buffers_&, hh_psc_alpha& ); //!<Sets buffer pointers to 0

    //! Logger for all analog data
    UniversalDataLogger< hh_psc_alpha > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spike_exc_;
    RingBuffer spike_inh_;
    RingBuffer currents_;

    /** GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

    // IntergrationStep_ should be reset with the neuron on ResetNetwork,
    // but remain unchanged during calibration. Since it is initialized with
    // step_, and the resolution cannot change after nodes have been created,
    // it is safe to place both here.
    double step_;            //!< step size in ms
    double IntegrationStep_; //!< current integration time step, updated by GSL

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double I_stim_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    /** initial value to normalise excitatory synaptic current */
    double PSCurrInit_E_;

    /** initial value to normalise inhibitory synaptic current */
    double PSCurrInit_I_;

    int RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< hh_psc_alpha > recordablesMap_;
};


inline port
hh_psc_alpha::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}


inline port
hh_psc_alpha::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

inline port
hh_psc_alpha::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

inline port
hh_psc_alpha::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
    throw UnknownReceptorType( receptor_type, get_name() );
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
hh_psc_alpha::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
hh_psc_alpha::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif // HAVE_GSL
#endif // HH_PSC_ALPHA_H
