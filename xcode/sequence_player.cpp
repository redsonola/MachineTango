/*
Copyright (c) 2015, Dimitri Diakopoulos All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "modernmidi.h"
#include "sequence_player.h"
#include "midi_message.h"
#include "midi_utils.h"
#include "timer.h"

using namespace mm;

MidiSequencePlayer::MidiSequencePlayer(MidiOutput & output) : shouldSequence(false), output(output)
{
    responder = NULL; 
}
    
MidiSequencePlayer::~MidiSequencePlayer()
{
    shouldSequence = false;
    loop = false;

    if (sequencerThread.joinable())
        sequencerThread.join();
    

}

double MidiSequencePlayer::ticksToSeconds(int ticks)
{
    double beats = (double) ticks / ticksPerBeat;
    double seconds = beats / (beatsPerMinute / 60.0);
    return seconds;
}

int MidiSequencePlayer::secondsToTicks(float seconds)
{
    double ticksPerSecond = (beatsPerMinute * ticksPerBeat) / 60.0;
    double ticks = ticksPerSecond * seconds;
    return (int) ticks;
}

void MidiSequencePlayer::loadSingleTrack(const MidiTrack & track, double ticksPerBeat, double beatsPerMinute)
{
    reset();

    this->ticksPerBeat = ticksPerBeat;
    this->beatsPerMinute = float(beatsPerMinute);
    msPerTick = 60000.0 / beatsPerMinute / ticksPerBeat;

    double localElapsedTicks = 0;

    // Events in track
    for (auto m : track)
    {
        localElapsedTicks += m->tick;
        double deltaTimestampInSeconds = ticksToSeconds( int(localElapsedTicks) );
//        if (m->m->getMessageType() == MessageType::NOTE_ON) addTimestampedEvent(0, deltaTimestampInSeconds, m); // already checks if non-meta message
    }
}

void MidiSequencePlayer::loadMultipleTracks(const std::vector<MidiTrack> & tracks, double ticksPerBeat, double beatsPerMinute)
{
    // Unimplemented
}

void MidiSequencePlayer::start()
{
    if (sequencerThread.joinable()) 
        sequencerThread.join();

    shouldSequence = true;
    sequencerThread = std::thread(&MidiSequencePlayer::run, this);

#if defined(MM_PLATFORM_WINDOWS)
    HANDLE threadHandle = sequencerThread.native_handle();
    int err = (int)SetThreadPriority(threadHandle, THREAD_PRIORITY_TIME_CRITICAL);
    if (err == 0)
    {
        std::cerr << "SetThreadPriority() failed: " << GetLastError() << std::endl;
    }

    err = (int)SetThreadAffinityMask(threadHandle, 1);
    if (err == 0)
    {
        std::cerr<< "SetThreadAffinityMask() failed: " << GetLastError() << std::endl;
    }
#endif
    
    sequencerThread.detach();

    if (startedEvent)
        startedEvent();
}
    
void MidiSequencePlayer::run()
{
    size_t eventCursor = 0;

    PlatformTimer timer;
    timer.start();
    mTimer.start();
    double lastTime = mTimer.getSeconds();


    

    while (eventCursor < eventList.size())
    {
        double res = bt->getTimeInSeconds() - lastTime;
//        std::cout << "Delta: " << res  << " ticksToSeconds(eventList[eventCursor].tick):" << ticksToSeconds(eventList[eventCursor].tick) << " eventCursor: " << eventCursor<< " eventList.size(): " << eventList.size() << std::endl;

        
        while(ticksToSeconds(eventList[eventCursor].tick) >= mTimer.getSeconds() - lastTime )
        {
//            continue;
//            std::cout << "Delta: " << bt->getTimeInSeconds() - lastTime  << " lastTime:" << lastTime<<  " bt->getTimeInSeconds():" << bt->getTimeInSeconds() << " ticksToSeconds(eventList[eventCursor].tick):" << ticksToSeconds(eventList[eventCursor].tick) << std::endl;

        }
        
        //std::cout << "eventList[eventCursor].tick: " << eventList[eventCursor].tick << " ticks per beat: " << ticksPerBeat << " bpm: " << beatsPerMinute << std::endl;;

        //TODO -- send to correct channel
        if(eventCursor < eventList.size())
        output.send(*eventList[eventCursor].msg.get());

        if (shouldSequence == false) 
            break;

        lastTime = mTimer.getSeconds() ;
        eventCursor++;
    }
    timer.stop();
    mTimer.stop();
        
    if (stoppedEvent){
        stoppedEvent();}
    if(responder != NULL){
//        std::cout << "sending stopped event\n";
        responder->playerStopped(getTag());
    }
}

void MidiSequencePlayer::stop()
{
    shouldSequence = false;
}
    
void MidiSequencePlayer::addTimestampedEvent(int track, double when, std::shared_ptr<MidiPlayerEvent> ev)
{
    if (ev->msg->isMetaEvent() == false)
    {
        eventList.push_back(*ev.get());
    }
}

float MidiSequencePlayer::length() const
{
    return playTimeSeconds;
}

void MidiSequencePlayer::setLooping(bool newState)
{
    loop = newState;
}

void MidiSequencePlayer::reset()
{
    eventList.clear();
    startTime = 0;
    playTimeSeconds = 0;
}
