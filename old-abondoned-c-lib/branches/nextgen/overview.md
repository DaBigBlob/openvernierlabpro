The application first calls `LabPro_init()` with a pointer to a `LabPro_context`.
`LabPro_context` is mostly a wrapper around a double-pointer to a `libusb_context`.
The application then calls `LabPro_get_devices()` to list the attached LabPro(s).
This returns a `LabPro_List` struct that contains a pointer to an array of `LabPro`s.

Additionally, the application may register with `LabPro_hotplug_register()` to get hotplug
events from libusb.

The application then calls `LabPro_open()` to attempt to open and claim a device from the
`LabPro_List`. This would be followed by `LabPro_query()`, which populates the `LabPro`
struct with a variety of information (such as battery level, firmware version, and
attached sensors). The application may want to periodically refresh the information by
calling `LabPro_query()` again.

