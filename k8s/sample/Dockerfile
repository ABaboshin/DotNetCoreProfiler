FROM mcr.microsoft.com/dotnet/sdk:6.0 as build

COPY src src

COPY k8s /k8s

RUN dotnet publish -c Release -o /app /k8s/sample/app/app.csproj

FROM mcr.microsoft.com/dotnet/aspnet:6.0

EXPOSE 80

WORKDIR /app

CMD ["dotnet", "app.dll"]

COPY --from=build /app /app

