using Interception.Observers.Configuration;
using Microsoft.Extensions.Configuration;
using System;
using System.Diagnostics;

namespace Interception.Observers
{
    public class EFCoreDiagnosticsObserver : IObserver<DiagnosticListener>
    {
        private readonly EntityFrameworkCoreConfiguration _entityFrameworkCoreConfiguration;

        public EFCoreDiagnosticsObserver(EntityFrameworkCoreConfiguration entityFrameworkCoreConfiguration)
        {
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
        }

        public static void ConfigureAndStart()
        {
            var configuration = new ConfigurationBuilder()
                .AddEnvironmentVariables()
                .Build();

            var entityFrameworkCoreConfiguration = configuration.GetSection(EntityFrameworkCoreConfiguration.SectionKey).Get<EntityFrameworkCoreConfiguration>();

            DiagnosticListener.AllListeners.Subscribe(new EFCoreDiagnosticsObserver(entityFrameworkCoreConfiguration));
        }
    }
}
