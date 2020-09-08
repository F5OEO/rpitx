"""Setup for rpitx."""

from setuptools import setup, Extension

setup(
    name='rpitx',
    version='0.2',
    description='Raspyberry Pi radio transmitter',
    author='Brandon Skari',
    author_email='brandon@skari.org',
    url='https://github.com/F5OEO/rpitx',
    ext_modules=[
        Extension(
            '_rpitx',
            [
                'src/RpiDma.c',
                'src/RpiGpio.c',
                'src/RpiTx.c',
                'src/mailbox.c',
                'src/python/_rpitxmodule.c',
                'src/raspberry_pi_revision.c',
                ],
            extra_link_args=['-lrt', '-lsndfile'],
            ),
        ],
    packages=['rpitx'],
    package_dir={'': 'src/python'},
    install_requires=[
        'ffmpegwrapper==0.1-dev',
        'pypi-libavwrapper',
        ],
    )
