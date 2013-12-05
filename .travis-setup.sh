#! /bin/bash

set -x
set -e

# Make sure /dev/shm has correct permissions.
ls -l /dev/shm
sudo chmod 1777 /dev/shm
ls -l /dev/shm

uname -a
cat /etc/lsb-release

sudo apt-get update --fix-missing

# Install python-imaging from the environment rather then build it.
# If the below fails, pip will build it via the requirements.txt
sudo apt-get install python-imaging
VIRTUAL_ENV_site_packages=$(echo $VIRTUAL_ENV/lib/*/site-packages)
VIRTUAL_ENV_python_version=$(echo $VIRTUAL_ENV_site_packages | sed -e's@.*/\(.*\)/site-packages@\1@')
ln -s /usr/lib/$VIRTUAL_ENV_python_version/dist-packages/PIL.pth $VIRTUAL_ENV_site_packages/PIL.pth
ln -s /usr/lib/$VIRTUAL_ENV_python_version/dist-packages/PIL $VIRTUAL_ENV_site_packages/PIL

export VERSION=$(echo $BROWSER | sed -e's/[^-]*-//')
export BROWSER=$(echo $BROWSER | sed -e's/-.*//')

echo BROWSER=$BROWSER
echo VERSION=$VERSION

sudo ln -sf $(which true) $(which xdg-desktop-menu)

case $BROWSER in
Chrome)
	echo "Getting $VERSION of $BROWSER"
	export CHROME=google-chrome-${VERSION}_current_amd64.deb
	wget https://dl.google.com/linux/direct/$CHROME
	sudo dpkg --install $CHROME || sudo apt-get -f install
	which google-chrome
	ls -l `which google-chrome`
	
	# If we don't have PID namespace support, download a custom
	# chrome-sandbox which works even without it.
	echo "Checking for namespace support, needed for sandboxing"
	NSTEST=/tmp/clone_namespaces_test
	gcc -o $NSTEST tools/clone_namespaces_test.c
	sudo chown root:root $NSTEST; sudo chmod 4755 $NSTEST
	if $NSTEST; then
		echo "No need to modify the sandbox."
	else
		if [ -f /opt/google/chrome/chrome-sandbox ]; then
			export CHROME_SANDBOX=/opt/google/chrome/chrome-sandbox
		else
			export CHROME_SANDBOX=$(ls /opt/google/chrome*/chrome-sandbox)
		fi
		
		sudo rm -f $CHROME_SANDBOX
		sudo wget https://googledrive.com/host/0B5VlNZ_Rvdw6NTJoZDBSVy1ZdkE -O $CHROME_SANDBOX
		sudo chown root:root $CHROME_SANDBOX; sudo chmod 4755 $CHROME_SANDBOX
		sudo ls -l $CHROME_SANDBOX
		sudo md5sum $CHROME_SANDBOX
	fi
	
	google-chrome --version --allow-sandbox-debugging
	;;

Firefox)
	sudo rm -f /usr/local/bin/firefox
	case $VERSION in
	beta)
		yes "\n" | sudo add-apt-repository -y ppa:mozillateam/firefox-next
		;;
	aurora)
		yes "\n" | sudo add-apt-repository -y ppa:ubuntu-mozilla-daily/firefox-aurora
		;;
	esac
	sudo apt-get update --fix-missing
	sudo apt-get install firefox
	which firefox
	ls -l `which firefox`
	firefox --version
	;;
esac

R=tools/python/requirements.txt
pip install -r $R --use-mirrors || pip install -r $R
