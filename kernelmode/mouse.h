#pragma once

#include "definitions.h"

// Original mouse simluation source: norsefireee
namespace mouse
{
	typedef VOID(*mouse_class_service_callback)(
		PDEVICE_OBJECT device_object,
		PMOUSE_INPUT_DATA input_data_start,
		PMOUSE_INPUT_DATA input_data_end,
		PULONG input_data_consumed
		);

	typedef struct _mouse_object
	{
		PDEVICE_OBJECT device_object;
		mouse_class_service_callback callback;
	} mouse_object, * pmouse_object;

	NTSTATUS initialize_object(pmouse_object object)
	{
		UNICODE_STRING class_driver_string;
		UNICODE_STRING hid_driver_string;
		PDRIVER_OBJECT class_driver_object;
		PDRIVER_OBJECT hid_driver_object;
		NTSTATUS status;

		RtlInitUnicodeString(&class_driver_string, L"\\driver\\mouclass");

		if (!NT_SUCCESS((status = ObReferenceObjectByName(&class_driver_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID *) &class_driver_object))))
			return status;

		RtlInitUnicodeString(&hid_driver_string, L"\\driver\\mouhid");

		if (!NT_SUCCESS((status = ObReferenceObjectByName(&hid_driver_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID *) &hid_driver_object))))
		{ 
			if (class_driver_object)
				ObDereferenceObject(class_driver_object);
			return status;
		}

		// Find devices and callbacks
		PDEVICE_OBJECT hid_device_object = hid_driver_object->DeviceObject;
		while (hid_device_object && !object->callback)
		{
			PDEVICE_OBJECT class_device_object = class_driver_object->DeviceObject;
			while (class_device_object && !object->callback)
			{
				if (!class_device_object->NextDevice && !object->device_object)
					object->device_object = class_device_object;

				PULONG_PTR device_ext = (PULONG_PTR) hid_device_object->DeviceExtension;
				ULONG_PTR device_ext_size = ((ULONG_PTR) hid_device_object->DeviceObjectExtension - (ULONG_PTR) hid_device_object->DeviceExtension) / 4;
								
				for (ULONG_PTR i = 0; i < device_ext_size; i++)
				{
					if (device_ext[i] == (ULONG_PTR) class_device_object && device_ext[i + 1] > (ULONG_PTR) class_driver_object)
					{
						object->callback = (mouse_class_service_callback)(device_ext[i + 1]);
						break;
					}
				}

				class_device_object = class_device_object->NextDevice;
			}

			hid_device_object = hid_device_object->NextDevice;
		}

		// If no device was asigned to the object, assign the last device for the target callback
		if (!object->device_object)
		{
			PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
			while (target_device_object)
			{
				if (!target_device_object->NextDevice)
				{
					object->device_object = target_device_object;
					break;
				}

				target_device_object = target_device_object->NextDevice;
			}
		}

		ObDereferenceObject(class_driver_object);
		ObDereferenceObject(hid_driver_object);
		return STATUS_SUCCESS;
	}

	void invoke_callback(mouse_object object, long offset_x, long offset_y, unsigned short flags, unsigned short button_flags)
	{
		ULONG consumed{0};
		MOUSE_INPUT_DATA data;
		RtlZeroMemory(&data, sizeof(MOUSE_INPUT_DATA));

		data.LastX = offset_x;
		data.LastY = offset_y;
		data.Flags = flags;
		data.ButtonFlags = button_flags;

		if (!object.callback)
		{
			DbgPrint("oreo.software: Attempted to simulate mouse input without a callback.\n");
			return;
		}

		if (!object.device_object)
		{
			DbgPrint("oreo.software: Attempted to simulate mouse input without a device.\n");
			return;
		}

		KIRQL irql;
		KeRaiseIrql(DISPATCH_LEVEL, &irql);
		object.callback(object.device_object, &data, (PMOUSE_INPUT_DATA) &data + 1, &consumed);
		KeLowerIrql(irql);
	}
}