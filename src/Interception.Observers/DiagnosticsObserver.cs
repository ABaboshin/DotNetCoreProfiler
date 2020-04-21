using Interception.Observers.Configuration;
using System;
using System.Diagnostics;

namespace Interception.Observers
{
    public class DiagnosticsObserver : IObserver<DiagnosticListener>
    {
        private readonly AspNetCoreConfiguration _aspNetCoreConfiguration;
        private readonly HttpHandlerConfiguration _httpHandlerConfiguration;

        public DiagnosticsObserver(AspNetCoreConfiguration aspNetCoreConfiguration, HttpHandlerConfiguration httpHandlerConfiguration)
        {
            _aspNetCoreConfiguration = aspNetCoreConfiguration;
            _httpHandlerConfiguration = httpHandlerConfiguration;
        }

        public void OnCompleted()
        {
        }

        public void OnError(Exception error)
        {
        }

        public void OnNext(DiagnosticListener value)
        {
            if (value.Name == "Microsoft.AspNetCore" && _aspNetCoreConfiguration.Enabled)
            {
                value.Subscribe(new AspNetCoreDiagnostics(_aspNetCoreConfiguration));
            }

            if (value.Name == "HttpHandlerDiagnosticListener" && _httpHandlerConfiguration.Enabled)
            {
                value.Subscribe(new HttpHandlerDiagnostrics(_httpHandlerConfiguration));
            }
        }
    }
}
