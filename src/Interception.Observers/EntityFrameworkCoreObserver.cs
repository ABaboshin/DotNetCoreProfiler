using Interception.Observers.Configuration;
using Interception.Tracing.Extensions;
using Microsoft.EntityFrameworkCore.Diagnostics;
using OpenTracing;
using OpenTracing.Tag;
using System;
using System.Collections.Generic;
using System.Threading;

namespace Interception.Observers
{
    public class EntityFrameworkCoreObserver : IObserver<KeyValuePair<string, object>>
    {
        private AsyncLocal<ISpan> _currentScope = new AsyncLocal<ISpan>();

        private readonly EntityFrameworkCoreConfiguration _configuration;

        public EntityFrameworkCoreObserver(EntityFrameworkCoreConfiguration configuration)
        {
            _configuration = configuration;
        }

        public void OnCompleted()
        {
        }

        public void OnError(Exception error)
        {
        }

        public void OnNext(KeyValuePair<string, object> kv)
        {
            // execution started
            if (kv.Key == RelationalEventId.CommandExecuting.Name && kv.Value is CommandEventData commandEventData)
            {
                _currentScope.Value = Interception.Tracing.Tracing.Tracer
                    .BuildSpan(_configuration.Name)
                    .WithTag(Tags.DbStatement, commandEventData.Command.CommandText)
                    .WithTag(Tags.SpanKind, Tags.SpanKindClient)
                    .AsChildOf(Interception.Tracing.Tracing.CurrentScope?.Span)
                    .Start();
            }

            if (kv.Key == RelationalEventId.CommandError.Name && kv.Value is CommandErrorEventData commandErrorEventData)
            {
                _currentScope.Value.SetException(commandErrorEventData.Exception);
                _currentScope.Value.Finish();
            }

            if (kv.Key == RelationalEventId.CommandExecuted.Name && kv.Value is CommandExecutedEventData commandExecutedEventData)
            {
                _currentScope.Value.Finish();
            }
        }
    }
}
