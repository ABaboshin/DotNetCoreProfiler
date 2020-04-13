using Interception.Observers.Configuration;
using Interception.Observers.Http;
using System;
using System.Diagnostics;

namespace Interception.Observers
{
    public class DiagnosticsObserver : IObserver<DiagnosticListener>
    {
        private readonly HttpConfiguration _httpConfiguration;

        public DiagnosticsObserver(HttpConfiguration httpConfiguration)
        {
            _httpConfiguration = httpConfiguration;
        }

        public void OnCompleted()
        {
        }

        public void OnError(Exception error)
        {
        }

        public void OnNext(DiagnosticListener value)
        {
            if (value.Name == "Microsoft.AspNetCore" && _httpConfiguration.Enabled)
            {
                value.Subscribe(new HttpObserver(_httpConfiguration));
            }
        }
    }
}
