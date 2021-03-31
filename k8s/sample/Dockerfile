FROM mcr.microsoft.com/dotnet/core/sdk:3.1.301 as build

COPY src src

COPY k8s /k8s

RUN dotnet publish -c Release -o /app /k8s/sample/app/app.csproj

FROM mcr.microsoft.com/dotnet/core/aspnet:3.1.5-bionic

EXPOSE 80

WORKDIR /app

CMD ["dotnet", "app.dll"]

COPY --from=build /app /app

