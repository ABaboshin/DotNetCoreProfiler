using Interception.Observers.Configuration;
using Interception.Tracing.Extensions;
using Microsoft.EntityFrameworkCore.Diagnostics;
using OpenTracing;
using OpenTracing.Tag;
using OpenTracing.Util;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

namespace Interception.Observers
{
    public class EntityFrameworkCoreObserver : BaseObserver
    {
        private AsyncLocal<ISpan> _currentScope = new AsyncLocal<ISpan>();

        private readonly EntityFrameworkCoreConfiguration _configuration;

        public EntityFrameworkCoreObserver(EntityFrameworkCoreConfiguration configuration)
        {
            _configuration = configuration;
        }

        public override void OnCompleted()
        {
        }

        public override void OnError(Exception error)
        {
        }

        public override void OnNext(KeyValuePair<string, object> kv)
        {
            // execution started
            if (kv.Key == RelationalEventId.CommandExecuting.Name && kv.Value is CommandEventData commandEventData)
            {
                _currentScope.Value = GlobalTracer.Instance
                    .BuildSpan(_configuration.Name)
                    .WithTag(Tags.DbStatement, commandEventData.Command.CommandText)
                    .WithTag(Tags.SpanKind, Tags.SpanKindClient)
                    .AsChildOf(GlobalTracer.Instance.ActiveSpan)
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

        public override bool ShouldSubscribe(DiagnosticListener diagnosticListener)
        {
            return diagnosticListener.Name == "Microsoft.EntityFrameworkCore" && _configuration.Enabled;
        }
    }
}
