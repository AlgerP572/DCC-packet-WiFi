/* Copyright (c) 2024 Alger Pike
    Same license terms as the derivative see below.

    This class based on the original string builder
    adds buffered logging capabiity for both Serial
    and Wifi logging.
*/

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

/*
 * StringBuilder class
 * 
 * Inherits from Print class so allows all Print class methods to write to 
 * a char[] buffer.
 * 
 */
#ifndef BufferedLogWriter_h
#define BufferedLogWriter_h

#include <Arduino.h>
#include <Print.h>

class BufferedLogWriter : public Print {
public:
    // Constructor
    BufferedLogWriter(char *buf, size_t len) :
        buffer(buf),
        buffMax(len - 1),
        _singleTaskSemaphore(xSemaphoreCreateMutex()), 
        _bufferSemaphore(xSemaphoreCreateMutex()), 
        _flushingBufferSemaphore(xSemaphoreCreateBinary()), 
        _flushingBufferCompleteSemaphore(xSemaphoreCreateBinary())       
    {        
        reset();
    }

    ~BufferedLogWriter()
    {
        if (_singleTaskSemaphore != nullptr)
        {
            vSemaphoreDelete(_singleTaskSemaphore);
            _singleTaskSemaphore = nullptr;
        }
        if (_bufferSemaphore != nullptr)
        {
            vSemaphoreDelete(_bufferSemaphore);
            _bufferSemaphore = nullptr;
        }
        if (_flushingBufferSemaphore != nullptr)
        {
            vSemaphoreDelete(_flushingBufferSemaphore);
            _flushingBufferSemaphore = nullptr;
        }
        if (_flushingBufferCompleteSemaphore != nullptr)
        {
            vSemaphoreDelete(_flushingBufferCompleteSemaphore);
            _flushingBufferCompleteSemaphore = nullptr;
        }
    }

    // Reset (empty) buffer.
    void reset() {
        buffIndex = 0;
        overflow = false;
        end();
    }

    // Pad with spaces to the nominated position.
    void setPos(unsigned int pos) {
        int n = pos - buffIndex;    // Number of spaces to write
        for (int i=0; i<n; i++)
            write(' ');
    }

    // Write a character.
    size_t write(uint8_t c) override
    {
        if (buffIndex + 3 < buffMax) {            
                buffer[buffIndex++] = c;
                if(c == '\n' || c == '\r')
                {
                    _newLine = true;
                }
            return 1;
        } else {
            overflow = true;
            return 0;
        }        
    }

    // Write an array of characters.
    size_t write(const uint8_t *buffer, size_t size) override
    {
        xSemaphoreTake(_singleTaskSemaphore, portMAX_DELAY);
        xSemaphoreTake(_bufferSemaphore, portMAX_DELAY);       

        OutputLog(size);        

		for (unsigned int i=0; i<size; i++)
        {
            if (!write(*buffer++))
                return i;
        }

		xSemaphoreGive(_bufferSemaphore);
        xSemaphoreGive(_singleTaskSemaphore);            
        return size;
    }

    void OutputLog(size_t size)
    {
        if((buffIndex + size) >= (buffMax - 4))
        {
            // If the next log info will fill the buffer
            // Print the existing log info and reset the
            // buffer.
            // The below happens in the task.
//            FlushLog();
            _bufferFull = true;

            xSemaphoreGive(_bufferSemaphore);
            xSemaphoreGive(_flushingBufferSemaphore);  
            xSemaphoreTake(_flushingBufferCompleteSemaphore, portMAX_DELAY);                   
            xSemaphoreTake(_bufferSemaphore, portMAX_DELAY);
        }
    }

    void FlushLog()
    {        
        if (xSemaphoreTake(_bufferSemaphore, portMAX_DELAY) == pdPASS) 
	    {          

            // Print current buffer contents.
            char* logString = getString();
            if(strlen(logString) <= 0)
            {
                // Empty: nothing to do.                
            }
            else
            {
                if(serialOn && _serial != nullptr)
                {
                    _serial->print(logString);
                }
                if(wifiOn && _wifi != nullptr)
                {
                    _wifi->print(logString);
                }
                reset();
            }

            if(_bufferFull == true)
            {
                _bufferFull = false;
                xSemaphoreGive(_flushingBufferCompleteSemaphore);
            }            
           
            xSemaphoreGive(_bufferSemaphore);            
	    }        
    }

    // Append a null character, to terminate the string 
    // in the buffer.  Position isn't updated, so further
    // characters can be appended to the string if required
    // (and if there's space).
    void end() {
        // There is still room for three characters plus a terminator.
        // If the buffer has filled, then add three dots.
        if (overflow && buffIndex+3 < buffMax) {
            buffer[buffIndex++] = '.';
            buffer[buffIndex++] = '.';
            buffer[buffIndex++] = '.';
        }
        buffer[buffIndex] = '\0'; // terminate string
    }

    // Get a pointer to the string within the buffer.
    char *getString() {
        end();
        return buffer;
    }

    void SetSerialOn()
    {
        serialOn = true;
    }

    void SetSerialOff()
    {
        serialOn = false;
    }

    void SetWifiOn()
    {
        wifiOn = true;
    }

    void SetWifiOff()
    {
        wifiOn = false;
    }

    bool GetNewLine()
    {
        return _newLine;
    }

    void SetNewLineFalse()
    {
        _newLine = false;
    }

    BaseType_t WaitBufferFull(TickType_t ticks)
    {        
        return xSemaphoreTake(_flushingBufferSemaphore, ticks);
    }

    void begin(Print* serial, Print* additionalPrint)
    {
        _serial = serial;
        _wifi = additionalPrint;
    }

private:
    char *buffer = nullptr;
    unsigned int buffMax = 0;
    unsigned int buffIndex = 0;
    bool overflow = false;    
    bool serialOn = true;
    bool wifiOn = true;
    bool _bufferFull = false;
    bool _newLine = false;

    SemaphoreHandle_t _singleTaskSemaphore = nullptr;
    SemaphoreHandle_t _bufferSemaphore = nullptr;
    SemaphoreHandle_t _flushingBufferSemaphore = nullptr;
    SemaphoreHandle_t _flushingBufferCompleteSemaphore = nullptr;

    Print* _serial = nullptr;
    Print* _wifi = nullptr;
};

#endif