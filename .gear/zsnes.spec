Name: zsnes           
Version: 2.0.10        
Release: alt1
Summary: Super Nintendo emulator      
Group: Amusements/Games
License: GPLv2       
URL: https://github.com/xyproto/zsnes           
Source0: %{name}-main.tar.gz    
BuildArch: x86_64   

BuildRequires: libSDL-devel
BuildRequires: libpng-devel
BuildRequires: nasm 
BuildRequires: gcc-c++
BuildRequires: i586-libSDL
BuildRequires: i586-libSDL-devel
BuildRequires: i586-zlib-devel
BuildRequires: i586-libglpng
BuildRequires: i586-libglpng-devel
BuildRequires: make


%description
ZSNES is Super Nintendo emulator. It works with 64-bit x86


%prep
%setup -q


%build
%make


%install
%makeinstall_std -C build

%files
%_bindir/*
%_libdir/*
%_mandir/man/%{name}.1




