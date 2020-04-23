using Interception.Observers.Configuration;
using Microsoft.Extensions.Configuration;
using System;
using System.Diagnostics;

namespace Interception.Observers
{
    public class DiagnosticsObserver : IObserver<DiagnosticListener>
    {
        private readonly AspNetCoreConfiguration _aspNetCoreConfiguration;
        private readonly HttpHandlerConfiguration _httpHandlerConfiguration;
        private readonly EntityFrameworkCoreConfiguration _entityFrameworkCoreConfiguration;

        public DiagnosticsObserver(AspNetCoreConfiguration aspNetCoreConfiguration, HttpHandlerConfiguration httpHandlerConfiguration, EntityFrameworkCoreConfiguration entityFrameworkCoreConfiguration)
        {
            _aspNetCoreConfiguration = aspNetCoreConfiguration;
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
            new AspNetCoreDiagnostics(_aspNetCoreConfiguration).Subscribe(diagnosticListener);
            new EntityFrameworkCoreObserver(_entityFrameworkCoreConfiguration).Subscribe(diagnosticListener);
            new HttpHandlerDiagnostrics(_httpHandlerConfiguration).Subscribe(diagnosticListener);
        }

        public static void ConfigureAndStart()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var aspNetCoreConfiguration = configuration.GetSection(AspNetCoreConfiguration.SectionKey).Get<AspNetCoreConfiguration>();
            var httpHandlerConfiguration = configuration.GetSection(HttpHandlerConfiguration.SectionKey).Get<HttpHandlerConfiguration>();
            var entityFrameworkCoreConfiguration = configuration.GetSection(EntityFrameworkCoreConfiguration.SectionKey).Get<EntityFrameworkCoreConfiguration>();

            DiagnosticListener.AllListeners.Subscribe(new DiagnosticsObserver(aspNetCoreConfiguration, httpHandlerConfiguration, entityFrameworkCoreConfiguration));
        }
    }
}
