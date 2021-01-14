from setuptools import setup, find_packages, Command
from setuptools.command.install import install
import distutils.command.install as orig
import subprocess
import inspect

class CustomInstall(install):

    user_options = install.user_options

    def initialize_options(self):
        install.initialize_options(self)

    def finalize_options(self):
        install.finalize_options(self)

    def run(self):

        # Copied from the original setup.command.install functions
        if not install._called_from_setup(inspect.currentframe()):
            # Run in backward-compatibility mode to support bdist_* commands.
            orig.install.run(self)
        else:
            install.do_egg_install()

        # and now adding in my own bit
        print("...building the Hector Translation code")
        subprocess.call(
            ['make', 
            '-C', 
            'hop/distortion_correction/HectorTranslationSoftware/Code']
        )
        print("...Done!")



setup(name='Hector-Observations-Pipeline',
      version='0.2.1',
      description='Hector Galaxy Survey Observations pipeline',
      author='Sam Vaughan',
      author_email='sam.vaughan@sydney.edu.au',
      url='',
      packages=find_packages(),
      include_package_data = True,
      cmdclass={'install': CustomInstall}
     )