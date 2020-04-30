using Interception.Observers.Configuration;
using Microsoft.Extensions.Configuration;
using System;
using System.Diagnostics;

namespace Interception.Observers
{
    public class DiagnosticsObserver : IObserver<DiagnosticListener>
    {
        private readonly HttpHandlerConfiguration _httpHandlerConfiguration;
        private readonly EntityFrameworkCoreConfiguration _entityFrameworkCoreConfiguration;

        public DiagnosticsObserver(HttpHandlerConfiguration httpHandlerConfiguration, EntityFrameworkCoreConfiguration entityFrameworkCoreConfiguration)
        {
            _httpHandlerConfiguration = httpHandlerConfiguration;
            _entityFrameworkCoreConfiguration = entityFrameworkCoreConfiguration;
        }

        public void OnCompleted()
        {
        }

        public void OnError(Exception error)
        {
        }

        public void OnNext(DiagnosticListener diagnosticListener)
        {
            new EntityFrameworkCoreObserver(_entityFrameworkCoreConfiguration).Subscribe(diagnosticListener);
            new HttpHandlerDiagnostrics(_httpHandlerConfiguration).Subscribe(diagnosticListener);
        }

        public static void ConfigureAndStart()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var httpHandlerConfiguration = configuration.GetSection(HttpHandlerConfiguration.SectionKey).Get<HttpHandlerConfiguration>();
            var entityFrameworkCoreConfiguration = configuration.GetSection(EntityFrameworkCoreConfiguration.SectionKey).Get<EntityFrameworkCoreConfiguration>();

            DiagnosticListener.AllListeners.Subscribe(new DiagnosticsObserver(httpHandlerConfiguration, entityFrameworkCoreConfiguration));
        }
    }
}
