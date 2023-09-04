## Process this file with automake to produce Makefile.in

bin_PROGRAMS = mpi_test_suite

EXTRA_DIST = cmdline.ggo

cmdline.c cmdline.h: cmdline.ggo
	gengetopt < $<

mpi_test_suite_SOURCES = \
	cmdline.c \
	cmdline.h \
	coll/tst_coll_allgather.c \
	coll/tst_coll_allgather_in_place.c \
	coll/tst_coll_allreduce.c \
	coll/tst_coll_allreduce_in_place.c \
	coll/tst_coll_allreduce_max.c \
	coll/tst_coll_allreduce_min.c \
	coll/tst_coll_allreduce_quadsum.c \
	coll/tst_coll_allreduce_sum.c \
	coll/tst_coll_alltoall.c \
	coll/tst_coll_bcast.c \
	coll/tst_coll_gather.c \
	coll/tst_coll_reduce_in_place_max.c \
	coll/tst_coll_reduce_in_place_min.c \
	coll/tst_coll_reduce_max.c \
	coll/tst_coll_reduce_min.c \
	coll/tst_coll_scan_sum.c \
	coll/tst_coll_scatter.c \
	coll/tst_coll_scatterv.c \
	coll/tst_coll_scatterv_stride.c \
	compile_info.h \
	dynamic/tst_comm_spawn.c \
	dynamic/tst_comm_spawn_multiple.c \
	dynamic/tst_establish_communication.c \
	env/tst_env_get_version.c \
	env/tst_env_request_null.c \
	env/tst_env_status_check.c \
	env/tst_env_type_dup.c \
	io/tst_file_append_mode.c \
	io/tst_file_asyncio_atomic.c \
	io/tst_file_io_atomic.c \
	io/tst_file_io_commself.c \
	io/tst_file_io_sync.c \
	io/tst_file_io_with_arrange.c \
	io/tst_file_io_with_hole2.c \
	io/tst_file_io_with_hole.c \
	io/tst_file_iread_at.c \
	io/tst_file_iread.c \
	io/tst_file_iread_shared.c \
	io/tst_file_iwrite_at.c \
	io/tst_file_iwrite.c \
	io/tst_file_iwrite_shared.c \
	io/tst_file_preallocate.c \
	io/tst_file_read_all_begin.c \
	io/tst_file_read_all.c \
	io/tst_file_read_at_all_begin.c \
	io/tst_file_read_at_all.c \
	io/tst_file_read_at.c \
	io/tst_file_read.c \
	io/tst_file_read_convert.c \
	io/tst_file_read_darray.c \
	io/tst_file_read_ordered_begin.c \
	io/tst_file_read_ordered.c \
	io/tst_file_read_shared.c \
	io/tst_file_read_subarray.c \
	io/tst_file_sequential_mode.c \
	io/tst_file_set_size.c \
	io/tst_file_simple.c \
	io/tst_file_write_all_begin.c \
	io/tst_file_write_all.c \
	io/tst_file_write_at_all_begin.c \
	io/tst_file_write_at_all.c \
	io/tst_file_write_at.c \
	io/tst_file_write.c \
	io/tst_file_write_darray.c \
	io/tst_file_write_ordered_begin.c \
	io/tst_file_write_ordered.c \
	io/tst_file_write_shared.c \
	io/tst_file_write_subarray.c \
	mpi_test_suite.c \
	mpi_test_suite.h \
	one-sided/tst_accumulate_with_fence_sum.c \
	one-sided/tst_accumulate_with_lock_max.c \
	one-sided/tst_accumulate_with_post_min.c \
	one-sided/tst_get_with_fence.c \
	one-sided/tst_get_with_lock.c \
	one-sided/tst_get_with_post.c \
	one-sided/tst_one_sided_simple_ring_get.c \
	one-sided/tst_one_sided_simple_ring_get_post.c \
	one-sided/tst_one_sided_simple_ring_put.c \
	one-sided/tst_put_with_fence.c \
	one-sided/tst_put_with_lock.c \
	one-sided/tst_put_with_post.c \
	p2p/tst_p2p_alltoall.c \
	p2p/tst_p2p_alltoall_graph.c \
	p2p/tst_p2p_alltoall_irsend.c \
	p2p/tst_p2p_alltoall_issend.c \
	p2p/tst_p2p_alltoall_persistent.c \
	p2p/tst_p2p_alltoall_probe_anysource.c \
	p2p/tst_p2p_alltoall_sendrecv.c \
	p2p/tst_p2p_alltoall_xisend.c \
	p2p/tst_p2p_direct_partner_intercomm.c \
	p2p/tst_p2p_many_to_one.c \
	p2p/tst_p2p_many_to_one_iprobe_anysource.c \
	p2p/tst_p2p_many_to_one_isend_cancel.c \
	p2p/tst_p2p_many_to_one_probe_anysource.c \
	p2p/tst_p2p_simple_ring_bottom.c \
	p2p/tst_p2p_simple_ring_bsend.c \
	p2p/tst_p2p_simple_ring.c \
	p2p/tst_p2p_simple_ring_ibsend.c \
	p2p/tst_p2p_simple_ring_irsend.c \
	p2p/tst_p2p_simple_ring_isend.c \
	p2p/tst_p2p_simple_ring_issend.c \
	p2p/tst_p2p_simple_ring_pack.c \
	p2p/tst_p2p_simple_ring_persistent.c\
	p2p/tst_p2p_simple_ring_rsend.c \
	p2p/tst_p2p_simple_ring_sendrecv.c \
	p2p/tst_p2p_simple_ring_shift.c \
	p2p/tst_p2p_simple_ring_ssend.c \
	p2p/tst_p2p_simple_ring_xsend.c \
	threaded/tst_threaded_comm_dup.c \
	threaded/tst_threaded_ring_bsend.c \
	threaded/tst_threaded_ring.c \
	threaded/tst_threaded_ring_isend.c \
	threaded/tst_threaded_ring_persistent.c \
	threaded/tst_threaded_ring_partitioned.c \
	threaded/tst_threaded_ring_partitioned_many_to_one.c \
	tst_comm.c \
	tst_comm.h \
	tst_file.c \
	tst_output.c \
	tst_output.h \
	tst_tests.c \
	tst_threads.c \
	tst_threads.h \
	tst_types.c

