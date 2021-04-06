using Interception.Observers.Configuration;
using Microsoft.Extensions.Configuration;
using System;
using System.Diagnostics;

namespace Interception.Observers
{
    public class HttpDiagnosticsObserver : IObserver<DiagnosticListener>
    {
        private readonly HttpHandlerConfiguration _httpHandlerConfiguration;

        public HttpDiagnosticsObserver(HttpHandlerConfiguration httpHandlerConfiguration)
        {
            _httpHandlerConfiguration = httpHandlerConfiguration;
        }

        public void OnCompleted()
        {
        }

        public void OnError(Exception error)
        {
        }

        public void OnNext(DiagnosticListener diagnosticListener)
        {
            new HttpHandlerDiagnostrics(_httpHandlerConfiguration).Subscribe(diagnosticListener);
        }

        public static void ConfigureAndStart()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var httpHandlerConfiguration = configuration.GetSection(HttpHandlerConfiguration.SectionKey).Get<HttpHandlerConfiguration>();

            DiagnosticListener.AllListeners.Subscribe(new HttpDiagnosticsObserver(httpHandlerConfiguration));
        }
    }
}
