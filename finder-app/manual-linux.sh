#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}
    # TODO: Add your kernel build steps here
    echo "Performing deep clean of the kernel build tree"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    echo "Configuring virt arm dev board "
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    echo "Building a kernel image for booting with QEMU"
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    echo "Building kernel modules"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    echo "Building device tree"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi 
echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}/
echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories

cd "$OUTDIR"
mkdir -p rootfs
cd rootfs
mkdir bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
   # TODO:  Configure busybox
    echo "Configuring busybox"
    make distclean
    make defconfig

else
    cd busybox
fi

# TODO: Make and install busybox
echo "Installing busybox"
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install # check if sudo is required

cd ${OUTDIR}/rootfs
echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"


# TODO: Add library dependencies to rootfs

#cp "${CROSS_COMPILE}gcc -print-sysroot"/lib/ld-linux-aarch64.so.1 lib
#cp "aarch64-none-linux-gnu-gcc -print-sysroot"/lib64/libm.so.6 lib64
#cp "aarch64-none-linux-gnu-gcc -print-sysroot"/lib64/libresolv.so.2 lib64
#cp "${CROSS_COMPILE}gcc -print-sysroot"/lib64/libc.so.6 lib64


cp $(${CROSS_COMPILE}gcc -print-sysroot)/lib/ld-linux-aarch64.so.1 lib
cp $(${CROSS_COMPILE}gcc -print-sysroot)/lib64/libm.so.6 lib64
cp $(${CROSS_COMPILE}gcc -print-sysroot)/lib64/libresolv.so.2 lib64
cp $(${CROSS_COMPILE}gcc -print-sysroot)/lib64/libc.so.6 lib64

# TODO: Make device nodes  
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1

# TODO: Clean and build the writer utility
cd ~/assignment-1-shreyan-collab/finder-app
make clean
make CROSS_COMPILE=${CROSS_COMPILE}

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs

cp ~/assignment-1-shreyan-collab/finder-app/{writer,finder.sh,finder-test.sh} ${OUTDIR}/rootfs/home
cp ~/assignment-1-shreyan-collab/finder-app/autorun-qemu.sh ${OUTDIR}/rootfs/home
cp ~/assignment-1-shreyan-collab/finder-app/conf/username.txt ${OUTDIR}/rootfs/home


# TODO: Chown the root directory

cd ${OUTDIR}/rootfs
sudo chown -R root:root *



# TODO: Create initramfs.cpio.gz

find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}
gzip -f initramfs.cpio
