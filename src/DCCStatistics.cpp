/* Copyright (c) 2021 Neil McKechnie
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

// Module containing singleton instance of DCCStatistics class.

#include "DCCStatistics.h"
#include "WifiSerialDebug.h"
#include "CpuUsage.h"

// Update statistics to reflect the received digital input transition.  Altbit
//  is zero if the transition is the end of the first half-bit and one if it is
//  the end of the second half of a DCC bit; Bitvalue is the value of the DCC
//  bit; and interruptInterval is the microsecond time between successive
//  interrupts (the length of the half-bit in DCC terms).
void INTERRUPT_SAFE DCCStatisticsClass::recordHalfBit(
    byte altbit, byte bitValue, unsigned int interruptInterval,
    unsigned int delta) {
  activeStats.count++;
  if (bitValue == 0) {
    activeStats.count0++;
    if (interruptInterval > activeStats.max0)
      activeStats.max0 = interruptInterval;
    if (interruptInterval < activeStats.min0)
      activeStats.min0 = interruptInterval;
    activeStats.total0 += interruptInterval;
    if (altbit && (delta > activeStats.max0BitDelta))
      activeStats.max0BitDelta = delta;
  } else {
    activeStats.count1++;
    if (interruptInterval > activeStats.max1)
      activeStats.max1 = interruptInterval;
    if (interruptInterval < activeStats.min1)
      activeStats.min1 = interruptInterval;
    activeStats.total1 += interruptInterval;
    if (altbit & (delta > activeStats.max1BitDelta))
      activeStats.max1BitDelta = delta;
  }
  if (interruptInterval < minBitLength)
    interruptInterval = minBitLength;
  else if (interruptInterval > maxBitLength)
    interruptInterval = maxBitLength;
  activeStats.countByLength[altbit][interruptInterval - minBitLength]++;
}

//=======================================================================
// WriteFullStatistics writes the statistics to Serial stream.
//
void DCCStatisticsClass::writeFullStatistics(Statistics &stats,
                                             bool showCpuStats,
                                             bool showBitLengths)
{

    // For clarity in the log forcing all of this information to be in the
    // same block of the log.
    Log::TakeMultiPrintSection();

    Log::print((char*)F("Bit Count/"), LogLevel::INFO);
    Log::print(refreshTime, LogLevel::INFO);
    Log::print((char*)F(" sec="), LogLevel::INFO);
    // These counts are for half-bits, so divide by two.
    Log::print((uint32_t)(stats.count / 2), LogLevel::INFO);
    Log::print((char*)F(" (Zeros="), LogLevel::INFO);
    Log::print((uint32_t)(stats.count0 / 2), LogLevel::INFO);
    Log::print((char*)F(", Ones="), LogLevel::INFO);
    Log::print((uint32_t) (stats.count1 / 2), LogLevel::INFO);
    Log::print((char*)F("), Glitches="), LogLevel::INFO);
    Log::println(&stats.glitchCount, LogLevel::INFO);

    Log::print((char*)F("Valid Packets="), LogLevel::INFO);
    Log::print(stats.packetCount, LogLevel::INFO);
    Log::print((char*)F(", NMRA out of spec="), LogLevel::INFO);
    Log::print(stats.outOfSpecRejectionCount, LogLevel::INFO);
    Log::print((char*)F(", Checksum Errors="), LogLevel::INFO);
    Log::print(stats.checksumError, LogLevel::INFO);
    Log::print((char*)F(", Lost pkts="), LogLevel::INFO);
    Log::print(stats.countLostPackets, LogLevel::INFO);
    Log::print((char*)F(", Long pkts="), LogLevel::INFO);
    Log::println(stats.countLongPackets, LogLevel::INFO);

    Log::print((char*)F("0 half-bit length (us): "), LogLevel::INFO);
    if (stats.min0 <= stats.max0) {
        Log::print((float)stats.total0 / stats.count0, LogLevel::INFO);
        Log::print((char*)F(" ("), LogLevel::INFO);
        Log::print(stats.min0, LogLevel::INFO);
        Log::print((char*)F("-"), LogLevel::INFO);
        Log::print(stats.max0, LogLevel::INFO);
        Log::print((char*)F(")"), LogLevel::INFO);
        Log::print((char*)F(" delta < "), LogLevel::INFO);
        Log::print((u32_t)stats.max0BitDelta, LogLevel::INFO);
    } else
        Log::print(F("<none>"), LogLevel::INFO);
    Log::println((const char*) "", LogLevel::INFO);

    Log::print((char*)F("1 half-bit length (us): "), LogLevel::INFO);
    if (stats.min1 <= stats.max1) {
        Log::print((float)stats.total1 / stats.count1, LogLevel::INFO);
        Log::print((char*)F(" ("), LogLevel::INFO);
        Log::print(stats.min1, LogLevel::INFO);
        Log::print((char*)F("-"), LogLevel::INFO);
        Log::print(stats.max1, LogLevel::INFO);
        Log::print((char*)F(")"), LogLevel::INFO);
        Log::print((char*)F(" delta < "), LogLevel::INFO);
        Log::print(stats.max1BitDelta, LogLevel::INFO);
    } else
        Log::print((char*)F("<none>"), LogLevel::INFO);
    Log::println((const char*) "", LogLevel::INFO);

    if (showCpuStats) {
        Log::print((char*)F("IRC Duration (us): "), LogLevel::INFO);
        if (stats.minInterruptTime <= stats.maxInterruptTime) {
            Log::print((float)stats.totalInterruptTime / stats.count, LogLevel::INFO);
            Log::print((char*)F(" ("), LogLevel::INFO);
            Log::print(stats.minInterruptTime, LogLevel::INFO);
            Log::print((char*)F("-"), LogLevel::INFO);
            Log::print(stats.maxInterruptTime, LogLevel::INFO);
            Log::print((char*)F(")"), LogLevel::INFO);
        } else 
        Log::print((char*)F("<none>"), LogLevel::INFO);

        // Calculate and display cpu load
        unsigned long spareLoopCountPerSec = stats.spareLoopCount / refreshTime;
        stats.cpuLoad = CpuUsage::GetCpuLoad(); //100.0f * (1.0f - (float)spareLoopCountPerSec / maxSpareLoopCountPerSec);
        Log::print((char*)F(",  CPU load: "), LogLevel::INFO);
        Log::print(
            stats.cpuLoad,
            LogLevel::INFO);
        Log::print((char*)F("%"), LogLevel::INFO);
        Log::println((const char*)"", LogLevel::INFO);
    }

    if (showBitLengths) {
        Log::println(F("------ Half-bit count by length (us) -------"), LogLevel::INFO);
        for (int i = minBitLength; i <= maxBitLength; i++) {
            unsigned long c0 = stats.countByLength[0][i - minBitLength];
            unsigned long c1 = stats.countByLength[1][i - minBitLength];
            if (c0 > 0 || c1 > 0) {
                if (i == minBitLength)
                    Log::print((char*)F("<="), LogLevel::INFO);
                else if (i == maxBitLength)
                    Log::print((char*)F(">="), LogLevel::INFO);
                Log::print(i, LogLevel::INFO);
                Log::print('\t', LogLevel::INFO);
                Log::print(&c0, LogLevel::INFO);
                Log::print('\t', LogLevel::INFO);
                Log::println(&c1, LogLevel::INFO);
            }
        }
        Log::println(F("--------------------------------------------"), LogLevel::INFO);    
    }

    // End of section to group together in log.
    Log::GiveMultiPrintSection();
}

// Return a copy of the current set of statistics accumulated.  We could inhibit
// interrupts while accessing the activeStats data, but the effect on the
// interrupt code may be more significant than the effect on the resulting
// counters.
Statistics DCCStatisticsClass::getAndClearStats()
{
  Statistics stats;
  memcpy(&stats, (void *)&activeStats, sizeof(activeStats));
  memset((void *)&activeStats, 0, sizeof(activeStats));
  activeStats.minInterruptTime = activeStats.min0 = activeStats.min1 = 65535;

  // Sample spare loop count and adjust max accordingly, for CPU load calcs.
  if (maxSpareLoopCountPerSec < stats.spareLoopCount / refreshTime)
    maxSpareLoopCountPerSec = stats.spareLoopCount / refreshTime;
  stats.refreshTime = refreshTime;
  return stats;
}

// Declare singleton class instance
DCCStatisticsClass DCCStatistics;
