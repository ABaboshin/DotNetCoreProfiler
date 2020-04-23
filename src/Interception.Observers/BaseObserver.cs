using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Interception.Observers
{
    public abstract class BaseObserver : IObserver<KeyValuePair<string, object>>
    {
        public void Subscribe(DiagnosticListener diagnosticListener)
        {
            if (ShouldSubscribe(diagnosticListener))
            {
                diagnosticListener.Subscribe(this);
            }
        }

        public abstract bool ShouldSubscribe(DiagnosticListener diagnosticListener);
        public abstract void OnCompleted();
        public abstract void OnError(Exception error);
        public abstract void OnNext(KeyValuePair<string, object> value);
    }
}
