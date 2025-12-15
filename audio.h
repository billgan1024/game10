vector<int> soundIndices;

// use another thread otherwise main thread will become slower
// auto audio = thread([]
void audioThread()
{

   int sampleRate = 48000;
   CoInitializeEx(null, 0);

   ComPtr<IMMDeviceEnumerator> mm;
   CoCreateInstance(__uuidof(MMDeviceEnumerator), null, CLSCTX_ALL, IID_PPV_ARGS(&mm));
   ComPtr<IMMDevice> dev;
   mm->GetDefaultAudioEndpoint(eRender, eConsole, &dev);

   ComPtr<IAudioClient> client;
   dev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, null, &client);
   // https://learn.microsoft.com/en-us/windows/win32/api/mmreg/ns-mmreg-waveformatex

   client->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY | AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 0, 0,
                      &WAVEFORMATEX{
                          .wFormatTag = WAVE_FORMAT_IEEE_FLOAT,
                          .nChannels = 2,
                          .nSamplesPerSec = u32(sampleRate),
                          .nAvgBytesPerSec = u32(sampleRate * 2 * sizeof(float)),
                          .nBlockAlign = 2 * sizeof(float),
                          .wBitsPerSample = 8 * sizeof(float),
                      },
                      null);

   u32 bufferSize;
   client->GetBufferSize(&bufferSize);
   ComPtr<IAudioRenderClient> render;
   client->GetService(IID_PPV_ARGS(&render));

   HANDLE ready = CreateEvent(null, false, false, null);
   client->SetEventHandle(ready);
   client->Start();

   // vec<float> sound = read_file<float>("audio/glass.bin");

   default_random_engine eng;
   auto rand = uniform_real_distribution<float>(-1, 1);

   auto sine = vector<float>(sampleRate);
   range (i, sampleRate)
   {
      sine[i] = sin(440 * 2 * pi * i / sampleRate);
   }

   while (true)
   {
      int res = WaitForSingleObject(ready, INFINITE);

      if (res == 0)
      {
         u32 padding;
         client->GetCurrentPadding(&padding);
         int n = bufferSize - padding;

         float* data;
         render->GetBuffer(n, (u8**)&data);

         range (i, n)
         {
            data[2 * i] = data[2 * i + 1] = 0;
            range (j, soundIndices.size())
            {
               if (soundIndices[j] < sine.size())
               {
                  data[2 * i] += sine[soundIndices[j]];
                  data[2 * i + 1] += sine[soundIndices[j]];
                  soundIndices[j]++;
               }
            }

            // data[2*i] = clamp(data[2*i], -1.0f, 1.0f);
            // data[2*i+1] = clamp(data[2*i+1], -1.0f, 1.0f);

            vector<int> newIndices;
            range (j, soundIndices.size())
            {
               if (soundIndices[j] < sine.size())
                  newIndices.push_back(soundIndices[j]);
            }
            soundIndices = newIndices;
         }

         render->ReleaseBuffer(n, 0);
      }
   }
}