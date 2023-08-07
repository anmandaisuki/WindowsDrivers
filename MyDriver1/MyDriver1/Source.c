#include "ntddk.h"

#define DEVICE_SEND CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801,METHOD_BUFFERED, FILE_WRITE_DATA)
#define DEVICE_REC CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802,METHOD_BUFFERED, FILE_READ_DATA)

UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\mydevice123");
UNICODE_STRING SymLinkName = RTL_CONSTANT_STRING(L"\\??\\mydevicelink123");

PDEVICE_OBJECT DeviceObject = NULL;

VOID Unload(PDRIVER_OBJECT DriverObject)
{
	IoDeleteSymbollicLink(&SymLinkName);
	IoDeleteDevice(DeviceObject);
	KdPrint(("Driver Unload.\r\n"));
}

NTSTATUS DispatchPassThru(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	switch (irpsp->MajorFunction) {
	case IRP_MJ_CREATE:
		KdPrint(("create request \r\n"));
		break;
	case IRP_MJ_CLOSE:
		KdPrint(("create request \r\n"));
		break;
	default:
		status = STATUS_INVALID_PARAMETER;
		break;
	}

	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

NTSTATUS DispatchDevCTL(PDEVICE_OBJECT DeviceObject, PIRP Irp) 
{
	PIO_STACK_LOCATION irpsp = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS status = STATUS_SUCCESS;

	PVOID buffer = Irp->AssociatedIrp.SystemBuffer;
	ULONG inLength = irpsp->Parameters.DeviceIoControl.InputBufferLength;

}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	NTSTATUS status;
	int i;

	DriverObject->DriverUnload = Unload;

	status = IoCreateDevice(DriverObject, 0, &DeviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	if (!NT_SUCCESS(status)) {
		KdPrint(("creating device failed \r\n"));
		return status;
	}

	status = IoCreateSymbolicLink(&SymLinkName, &DeviceName);

	if (!NT_SUCCESS(status)) {
		KdPrint(("creating symlink failed \r\n"));
		IoDeleteDevice(DeviceObject);
		return status;
	}
	
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = DispatchPassThru;
	}
	
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchDevCTL;
	//DriverObject->MajorFunction[IRP_MJ_WRITE] = DispatchCustom1;
	
	KdPrint(("Driver load. \r\n"));

		return status;
}