# NAME

Bedrock::Apache::API - Base class for implementing modular, method-validated APIs in mod\_perl

# SYNOPSIS

In your API class:

    package MyApp::API;

    use parent qw(Bedrock::Apache::API);

    our $VERSION = '1.2.3';

    sub api_version {
      my ($self) = @_;

      my $r = $self->get_request;

      my $dispatch = {
        GET => sub {
          $r->content_type('text/plain');
          $r->print($VERSION);
        },
      };

      return $dispatch->{$method}->();
    }

In your configuration file:

    <object name="routes">
      <array name="version">
        <scalar>GET</scalar>
      </array>
    </object>

See [Bedrock::Apache::API::Routes](https://metacpan.org/pod/Bedrock%3A%3AApache%3A%3AAPI%3A%3ARoutes) for an example API.

# DESCRIPTION

`Bedrock::Apache::API` provides a base class building REST-style APIs
under mod\_perl. You this module subclass, define endpoints (e.g.,
`version`, `status`) and register supported HTTP methods per
endpoint. Then implement your endpoints.

The base class takes care of:

\- Parsing parameters and cookies
\- Session integration
\- Response rendering (typically JSON)
\- Validating that only declared methods are used
\- Storing configuration
\- Authenticating requests against an API key

# USAGE OVERVIEW

- Subclass `Bedrock::Apache::API`
- Add routes to a configuration file
- For each endpoint, implement `api_{name}()` and route by method

# CONSTRUCTOR

## new( \\%options )

Instantiates the API object.

Supported keys:

- `request` - Apache2::RequestRec object
- `config` - Optional config hashref
- `params` - Optional parsed params
- `base_url` - Optional base path
- `autoconnect` - Connect to DB automatically
- `use_sessions` - Enable session support
- `cookieless_sessions` - Allow URL token fallback

# METHODS AND SUBROUTINES

The `init_api` and `init_routes` methods should be mostly opaque to
you the REST API writer. They are documented here for completeness.

## init\_api

Called automatically by `new()` to bootstrap configuration, context, params,
DBI connection, and session.

## init\_routes( \\%endpoint\_to\_methods )

Intializes the list of valid endpoints and HTTP methods. See ["ROUTE
CONFIGURATION"](#route-configuration) for more details.

## get\_param(param-name)

Returns the decoded query or POST parameter.

## get\_cookie(cookie-name)

Returns a cookie value.

## render\_response($data)

Outputs a data structure (hashref or arrayref) as a JSON response with the
appropriate content-type header.

## show\_error($message)

Writes the error to Apache's log and sends a 500 error with error
message.

## get\_request

Returns the Apache2::RequestRec object.

## set\_api\_config( \\%config )

Stores a config hash in the object for use by handlers.

## get\_config

Retrieves the config set via `set_api_config`.

## get\_context

Returns the Bedrock::Context object associated with the request.

# ROUTE CONFIGURATION

    Typically routes are defined in your API's configuration file like this:

     <object name="routes">
       <array name="version">
         <scalar>GET</scalar>
       </array>
       <array name="customer">
         <scalar>GET</scalar>
         <scalar>POST</scalar>
       </array>
     </object>
     <array name="protected">
       <scalar>customer</scalar>
     </array>

Each route should define an array of HTTP methods that endpoint will
support. You then implement a method with the naming convention
`api_{route}`. 

_If your route is more complex, for example `/contacts/officers` the
endpoint you would define is still `customer`. That is, you must
handle more complex routes inside your endpoint's method._

## Authenticating Requests

You can create an optional `protected` array that defines routes that
cause the `authenticate_request()` method to be called.  The default
behavior will check the `X-API-KEY` header against the value of
`API_KEY` that you define in your configuration file.  If the values
do not match, an 401 (UNAUTHORIZED) will will be returned.

Override the `authenticate_request()` method if you want more robust
authentication or authorization. The method should return a true value
if the request should be allowed.

This method is called during the construction of the object. If you
want to override this method you must set the routes with the
`set_routes()` method.

Example:

    $self->set_routes({
      ping    => [qw(GET)],
      login   => [qw(POST)],
      profile => [qw(GET POST PUT)],
    });

# API METHOD CONVENTION

For each key in your `routes` configuration array, you must implement
a method called:

    api_{key}

Inside this method, you should dispatch to another method or
subroutine based on the HTTP method:

    sub api_profile {
      my ($self) = @_;

      my $method = $self->get_request->method;

      return {
        GET => sub { ... },
        POST => sub { ... },
      }->{$method}->($self);
    }

The parent class will automatically return 400 if the requested method
was not listed in your route table.

# SEE ALSO

[Apache2::Const](https://metacpan.org/pod/Apache2%3A%3AConst), [Bedrock::Apache::ExampleAPI](https://metacpan.org/pod/Bedrock%3A%3AApache%3A%3AExampleAPI), [Bedrock::Apache::Handler](https://metacpan.org/pod/Bedrock%3A%3AApache%3A%3AHandler), [Apache2::RequestRec](https://metacpan.org/pod/Apache2%3A%3ARequestRec)

# AUTHORS

Rob Lauer - <rlauer6@comcast.net>

# LICENSE

This module is released under the same terms as Perl itself.
