#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <librpitx/librpitx.h>

int FileFreqTiming;

ngfmdmasync *fmmod;
static double GlobalTuningFrequency = 00000.0;
int FifoSize = 10000; // 10ms
double frequencyshift = 20000;
bool running = true;

void playtone(double Frequency, uint32_t Timing) // Timing in 0.1us
{
  uint32_t SumTiming = 0;
  SumTiming += Timing % 100;
  if (SumTiming >= 100) {
    Timing += 100;
    SumTiming = SumTiming - 100;
  }
  int NbSamples = (Timing / 100);

  while (NbSamples > 0) {
    usleep(10);
    int Available = fmmod->GetBufferAvailable();
    if (Available > FifoSize / 2) {
      int Index = fmmod->GetUserMemIndex();
      if (Available > NbSamples)
        Available = NbSamples;
      for (int j = 0; j < Available; j++) {
        fmmod->SetFrequencySample(Index + j, Frequency);
        NbSamples--;
      }
    }
  }
}

static void SendTones() {
  double basefreq = 1100;
  double freq2 = 1100 + frequencyshift;
  while (running) {
    playtone(1100, 10000000);
    playtone(freq2, 10000000);
  }
}

static void terminate(int num) {
  running = false;
  fprintf(stderr, "Caught signal - Terminating %x\n", num);
}

int main(int argc, char **argv) {
  float frequency = 144.5e6;
  if (argc > 1) {
    frequency = atof(argv[1]);
    frequencyshift = atof(argv[2]);
  } else {
    printf("usage : foxhunt frequency(Hz) frequency shift(Hz)\n");
    exit(0);
  }

  for (int i = 0; i < 64; i++) {
    struct sigaction sa;

    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = terminate;
    sigaction(i, &sa, NULL);
  }

  fmmod = new ngfmdmasync(frequency, 100000, 14, FifoSize);
  SendTones();
  delete fmmod;
  return 0;
}
