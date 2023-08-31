#include <pybind11/pybind11.h>

#include <cstdlib>
#include <iostream>
#include <nlohmann/json.hpp>
#include <unordered_map>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

using namespace nlohmann;
namespace py = pybind11;

std::string extract_info(std::string file_path) {
	json result = json::array();
	int ret;
	AVFormatContext *input_ctx = nullptr;

	ret = avformat_open_input(&input_ctx, file_path.c_str(), nullptr, nullptr);
	if (ret < 0) {
		std::cerr << "Could not open input file" << std::endl;
		return NULL;
	}

	for (unsigned int i = 0; i < input_ctx->nb_streams; i++) {
		json current = json::object();

		AVStream *stream = input_ctx->streams[i];

		std::string codec_type(av_get_media_type_string(stream->codecpar->codec_type));
		std::string codec_id(avcodec_descriptor_get(stream->codecpar->codec_id)->long_name);

		current["index"] = stream->index;
		current["codec_type"] = codec_type;
		current["codec_id"] = codec_id;

		AVDictionaryEntry *tag = nullptr;
		if ((tag = av_dict_get(stream->metadata, "language", tag, AV_DICT_IGNORE_SUFFIX))) {
			current["language"] = tag->value;
		} else {
			current["language"] = "und";
		}

		result.push_back(current);
	}

	avformat_close_input(&input_ctx);
	return result.dump();
}

bool extract_stream(std::string file_path, int stream_index, std::string output_path) {
	int ret;
	AVFormatContext *input_ctx = nullptr;

	ret = avformat_open_input(&input_ctx, file_path.c_str(), nullptr, nullptr);
	if (ret < 0) {
		std::cerr << "Could not open input file" << std::endl;
		return false;
	}

	AVStream *stream = input_ctx->streams[stream_index];

	AVFormatContext *output_ctx = nullptr;
	ret = avformat_alloc_output_context2(&output_ctx, nullptr, nullptr, output_path.c_str());
	if (ret < 0) {
		std::cerr << "Could not allocate output context" << std::endl;
		avformat_close_input(&input_ctx);
		return EXIT_FAILURE;
	}

	AVStream *out_stream = avformat_new_stream(output_ctx, nullptr);
	if (!out_stream) {
		std::cerr << "Could not create new stream" << std::endl;
		avformat_close_input(&input_ctx);
		avformat_free_context(output_ctx);
		return EXIT_FAILURE;
	}

	ret = avcodec_parameters_copy(out_stream->codecpar, stream->codecpar);
	if (ret < 0) {
		std::cerr << "Could not copy codec parameters" << std::endl;
		avformat_close_input(&input_ctx);
		avformat_free_context(output_ctx);
		return EXIT_FAILURE;
	}

	ret = avio_open(&output_ctx->pb, output_path.c_str(), AVIO_FLAG_WRITE);
	if (ret < 0) {
		std::cerr << "Could not open output file" << std::endl;
		avformat_close_input(&input_ctx);
		avformat_free_context(output_ctx);
		return EXIT_FAILURE;
	}

	ret = avformat_write_header(output_ctx, nullptr);
	if (ret < 0) {
		std::cerr << "Error writing header" << std::endl;
		avio_closep(&output_ctx->pb);
		avformat_free_context(output_ctx);
		return EXIT_FAILURE;
	}

	AVPacket packet;
	while (av_read_frame(input_ctx, &packet) >= 0) {
		if (packet.stream_index == stream_index) {
			packet.stream_index = out_stream->index;
			ret = av_interleaved_write_frame(output_ctx, &packet);
			if (ret < 0) {
				std::cerr << "Error writing packet" << std::endl;
				break;
			}
		}

		av_packet_unref(&packet);
	}

	ret = av_write_trailer(output_ctx);
	if (ret < 0) {
		std::cerr << "Error writing trailer" << std::endl;
		return EXIT_FAILURE;
	}

	avio_closep(&output_ctx->pb);
	avformat_free_context(output_ctx);
	avformat_close_input(&input_ctx);

	return true;
}

PYBIND11_MODULE(subextractor, module_handle) {
	module_handle.doc() = "Subtitle Extractor";
	module_handle.def("extract_info", &extract_info);
	module_handle.def("extract_stream", &extract_stream);
}