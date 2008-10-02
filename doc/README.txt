
NB: gotchas:


X Properties
------------

The gx_window_get/set_property functions are awkward since gobjects use such
names to set object properties, but in X11 terms they would send and retrieve
X Window properties to/from the xserver.

Currently gx_window_get/set_property correspond to the gobject varient and we
have gx_window_get/set_xproperty for the X11 kind.

The xproto defines an enum that should be named GXWindowClass, but that
conflicts with the the class definition for GXWindow objects.


GXWindowClass
-------------
Currently the gobject variant has been renamed _GXWindowClass, but given that
the GXWindowClass enum will be used quite a bit, I wonder if that might be a
bit confusing?


Errors
------
The recomended way to handle errors is by passing in a GError pointer to the
request functions, or if you are using the *_async requests, then pass the
pointer to the corresponding *_reply call.

TODO:
To support Xlib event handling semantics though, you can instead pass NULL for
the GError pointers and connect to the error signal on the GXConnection object.

If you are using GErrors, then is also worth being aware that if you are using
the synchronous request APIs and in particular use a request that doesn't have a
corresponding reply, then passing a non NULL GError means that function will
block until it knows if the request generates an error. (Similar to those
requests that do have a reply, where it also has to block) If you don't care
about the error then by passing NULL, the function can return immediatly after
sending the request. If you don't want the blocking but do care about the error
you should be using the *_async API.

