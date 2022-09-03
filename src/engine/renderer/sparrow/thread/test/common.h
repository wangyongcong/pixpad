#pragma once

struct message {
	unsigned long m_id;
	unsigned long m_data;
	message(unsigned long id = 0, unsigned long data = 0) : m_id(id), m_data(data) {}
};

struct
{
	unsigned long m_count;
	unsigned long m_sum;
} g_result = { 0, 0 };

void busy(const message &msg)
{
	g_result.m_count += 1;
	g_result.m_sum += msg.m_data;
}

const unsigned long N = 1024 * 1024 * 2;

const unsigned long QUEUE_SIZE = 256;

