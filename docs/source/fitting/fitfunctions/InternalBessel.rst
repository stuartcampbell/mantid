.. _func-InternalBessel:

==============
InternalBessel
==============

.. index:: InternalBessel

Description
-----------

A bessel function with fast damped oscillation and slow decay that could apply to incommensurate magnetic structures or spin density waves.

.. math:: A(t)= \alpha_\text{fast}J_0(2\pi\nu t+\phi)e^{{-\lambda_ft}^{\beta_f}}+\alpha_\text{slow}e^{{-\lambda_st}^{\beta_s}}

where,
 
:math:`\alpha_\text{fast}` is the asymmetry of the fast part,

:math:`\alpha_\text{slow}` is the asymmetry due to slow part,

:math:`J_0(x)` is the Bessel function of the first kind,

:math:`\lambda_\text{fast}` is damping rate of fast part,

:math:`\lambda_\text{slow}` is decay rate of slow part,

:math:`\nu` (MHz) is the frequency,

:math:`\phi` is the phase at time :math:`t=0`.

.. figure:: /images/InternalBessel.png

.. attributes::

.. properties::

References
----------

[1]  `K. Papadopoulos, Master thesis p.17 (2019) <http://uu.diva-portal.org/smash/get/diva2:1314127/FULLTEXT01.pdf>`_.

.. categories::

.. sourcelink::
