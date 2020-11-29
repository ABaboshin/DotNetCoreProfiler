using System;
using Interception.Attributes;

namespace SampleAttributes
{
  /// <summary>
  /// sample attribute
  /// </summary>
  [AttributeUsage(AttributeTargets.Method, AllowMultiple = false, Inherited = false)]
  public class MySampleAttribute : MethodInterceptorAttribute
  {
    /// <summary>
    /// dummy parameter
    /// </summary>
    public int P1 { get; set; }

    /// <summary>
    /// another dummy parameter
    /// </summary>
    public string[] P2 { get; set; }
  }
}
