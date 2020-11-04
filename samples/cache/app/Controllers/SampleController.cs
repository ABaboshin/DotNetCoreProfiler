using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Interception.Attributes;
using Interception.Attributes.Cache;

namespace app.Controllers
{
  [Route("api/[controller]")]
  [ApiController]
  public class SampleController : ControllerBase
  {
    [HttpGet]
    public ActionResult<int> Get()
    {
      return Ok(Fibonacci30(30));
    }

    [HttpGet("invalidate")]
    public ActionResult<int> Invalidate()
    {
      InvalidateCache();
      return Ok(0);
    }

    [InvalidateCache(Name = "Fibonacci30-30")]
    void InvalidateCache()
    {
    }

    [Cache(DurationSeconds = 6000, Parameters = new[] { "n" })]
    int Fibonacci30(int n)
    {
      return Fibonacci(30);
    }

    int Fibonacci(int n)
    {
      if (n < 0)
      {
        throw new ArgumentOutOfRangeException();
      }

      if (n == 0)
      {
        return 0;
      }

      if (n == 1)
      {
        return 1;
      }

      return Fibonacci(n - 1) + Fibonacci(n - 2);
    }
  }
}
