"""Setup for rpitx."""

from setuptools import setup, Extension

setup(
    name='rpitx',
    version='0.1',
    description='Raspyberry Pi radio transmitter',
    author='Brandon Skari',
    author_email='brandon@skari.org',
    url='https://github.com/F5OEO/rpitx',
    ext_modules=[
        Extension(
            '_rpitx',
            [
                'src/python/_rpitxmodule.c',
                'src/RpiTx.c',
                'src/mailbox.c',
                'src/RpiDma.c',
                'src/RpiGpio.c',
            ],
            extra_link_args=['-lrt', '-lsndfile'],
        ),
    ],
    package_dir={'': 'src/python'},
    install_requires=['pydub', 'wave'],
)
