#ifndef SMP_LOGGER_HPP
#define SMP_LOGGER_HPP

#include "ewts/module_constants.hpp"

// Provide the constant in the global namespace
inline constexpr const char* EWTS_ID_SMP = ewts::modules::EWTS_ID_SMP;

// Bind this module's logger identity
#define EWTS_ID EWTS_ID_SMP

#include "ewts/logger.hpp"

using ewts::EwtsInit;
using ewts::LogLevel;
using ewts::GetLogLevel;


#endif /* SMP_LOGGER_HPP */
