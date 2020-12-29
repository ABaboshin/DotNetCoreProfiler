using Interception.Attributes.Cache;
using Interception.Attributes;
using Microsoft.AspNetCore.Mvc;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Net.Http;
using System.Threading.Tasks;
using System;

namespace app.Controllers
{
  [Route("api/[controller]")]
  [ApiController]
  public class SampleController : ControllerBase
  {
    [HttpGet]
    public async Task<ActionResult> Get()
    {
      var client = new HttpClient();

      var request = new HttpRequestMessage
      {
          Method = HttpMethod.Get,
          RequestUri = new Uri($"{Environment.GetEnvironmentVariable("SERVICE_URL")}/api/sample/test")
      };

      var response = await client.SendAsync(request);

      return Ok();
    }

    [HttpGet("test")]
    public async Task<ActionResult> Test()
    {
      await Task.Delay(3000);

      return Ok();
    }
  }
}
