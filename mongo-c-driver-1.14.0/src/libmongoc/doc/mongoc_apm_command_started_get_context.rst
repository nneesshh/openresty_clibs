:man_page: mongoc_apm_command_started_get_context

mongoc_apm_command_started_get_context()
========================================

Synopsis
--------

.. code-block:: c

  void *
  mongoc_apm_command_started_get_context (
     const mongoc_apm_command_started_t *event);

Returns this event's context.

Parameters
----------

* ``event``: A :symbol:`mongoc_apm_command_started_t`.

Returns
-------

The pointer passed with :symbol:`mongoc_client_set_apm_callbacks` or :symbol:`mongoc_client_pool_set_apm_callbacks`.

See Also
--------

:doc:`Introduction to Application Performance Monitoring <application-performance-monitoring>`

