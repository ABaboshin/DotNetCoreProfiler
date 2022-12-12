using System;
using System.Collections.Generic;
using System.Diagnostics.Tracing;
using System.Threading;

namespace Interception.Observers.Samplers
{
    public class GCEventListener : EventListener
    {
        private int _inducedCount = 0;
        private readonly int[] _collectionCountPerGen = new int[GC.MaxGeneration + 1];

        private object _gen0Size = 0UL;
        private object _gen0Promoted = 0UL;
        private object _gen1Size = 0UL;
        private object _gen1Promoted = 0UL;
        private object _gen2Size = 0UL;
        private object _gen2Promoted = 0UL;
        private object _lohSize = 0UL;
        private object _lohPromoted = 0UL;
        private object _handlesCount = 0UL;
        

        public GCEventListener()
        {
            for (int i = 0; i < _collectionCountPerGen.Length; i++)
            {
                _collectionCountPerGen[i] = 0;
            }
        }

        protected override void OnEventSourceCreated(EventSource eventSource)
        {
            if (eventSource.Name.Equals("Microsoft-Windows-DotNETRuntime"))
            {
                EnableEvents(eventSource, EventLevel.Informational, (EventKeywords)1L);
                base.OnEventSourceCreated(eventSource);
            }
        }

        protected override void OnEventWritten(EventWrittenEventArgs eventData)
        {
            switch (eventData.EventId)
            {
                case 1:
                    Process_GCStart(eventData);
                    break;
                case 4:
                    Process_GCHeapStat(eventData);
                    break;
            }
        }

        private void Process_GCHeapStat(EventWrittenEventArgs eventData)
        {
            // https://learn.microsoft.com/en-us/dotnet/fundamentals/diagnostics/runtime-garbage-collection-events#gcheapstats_v2-event
            Interlocked.Exchange(ref this._gen0Size, eventData.Payload[0]);
            Interlocked.Exchange(ref this._gen0Promoted, eventData.Payload[1]);
            Interlocked.Exchange(ref this._gen1Size, eventData.Payload[2]);
            Interlocked.Exchange(ref this._gen1Promoted, eventData.Payload[3]);
            Interlocked.Exchange(ref this._gen2Size, eventData.Payload[4]);
            Interlocked.Exchange(ref this._gen2Promoted, eventData.Payload[5]);
            Interlocked.Exchange(ref this._lohSize, eventData.Payload[6]);
            Interlocked.Exchange(ref this._lohPromoted, eventData.Payload[7]);
            Interlocked.Exchange(ref this._handlesCount, eventData.Payload[12]);
        }

        private void Process_GCStart(EventWrittenEventArgs eventData)
        {
            // https://learn.microsoft.com/en-us/dotnet/fundamentals/diagnostics/runtime-garbage-collection-events#gcstart_v2-event
            switch ((uint)eventData.Payload[2])
            {
                case 1:
                case 7:
                    Interlocked.Increment(ref _inducedCount);
                    break;
            }

            for (int i = 0; i < (uint) eventData.Payload[1]; i++)
            {
                Interlocked.Increment(ref _collectionCountPerGen[i]);
            }
        }

        public Dictionary<string, object> Sample()
        {
            var result = new Dictionary<string, object>()
            {
            {
                "Gen0Size",
                _gen0Size
            },
            {
                "Gen0Promoted",
                _gen0Promoted
            },
            {
                "Gen1Size",
                _gen1Size
            },
            {
                "Gen1Promoted",
                _gen1Promoted
            },
            {
                "Gen2Size",
                _gen2Size
            },
            {
                "Gen2Survived",
                _gen2Promoted
            },
            {
                "LOHSize",
                _lohSize
            },
            {
                "LOHSurvived",
                _lohPromoted
            },
            {
                "HandlesCount",
                _handlesCount
            },
            {
                "InducedCount",
                _inducedCount
            }
            };

            Interlocked.Exchange(ref _inducedCount, 0);
            if (_collectionCountPerGen.Length != 0)
            {
                result["Gen0CollectionCount"] = _collectionCountPerGen[0];
                Interlocked.Exchange(ref _collectionCountPerGen[0], 0);
            }
                
            if (_collectionCountPerGen.Length > 1)
            {
                result["Gen1CollectionCount"] = _collectionCountPerGen[1];
                Interlocked.Exchange(ref _collectionCountPerGen[1], 0);
            }
                
            if (_collectionCountPerGen.Length > 2)
            {
                result["Gen2CollectionCount"] = _collectionCountPerGen[2];
                Interlocked.Exchange(ref _collectionCountPerGen[2], 0);
            }
                
            return result;
        }
    }
}
