#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "PNGStructure.h"
#include "Main.h"

#define Consume(type, streaming_buffer) *(type*)(streaming_buffer)->at; \
Consume_(streaming_buffer, sizeof(type)) 

internal void
Consume_(StreamingBuffer* streaming_buffer, size_t size)
{
    streaming_buffer->size -= size;
    Assert(size >= 0);
    streaming_buffer->at += size;
}

internal void
Consume_(StreamingIDATChunk* streaming_idat_chunk, size_t size)
{
    s32 size_difference = streaming_idat_chunk->size - size;
    if(size_difference >= 0)
    {
        streaming_idat_chunk->size -= size;
        streaming_idat_chunk->at += size;
        if(size_difference == 0 && streaming_idat_chunk->next)
        {
            streaming_idat_chunk->next->bits = streaming_idat_chunk->bits;
            *streaming_idat_chunk = *streaming_idat_chunk->next;
        }
    }
    else
    {
        Assert(false);
    }
}

internal u32
ConsumeBits(StreamingIDATChunk* streaming_idat_chunk, u32 bit_count)
{
    Assert(bit_count <= BITS_IN_TWO_BYTES);
    
    u32 result = 0;
    if(bit_count)
    {
        if((streaming_idat_chunk->bit_count + bit_count) <= BITS_IN_TWO_BYTES)
        {
            u32 start = streaming_idat_chunk->bit_count;
            u32 end = start + bit_count - 1;
            result = GetBits(streaming_idat_chunk->bits, start, end);
            streaming_idat_chunk->bit_count += bit_count;
            
            if(streaming_idat_chunk->bit_count == BITS_IN_TWO_BYTES)
            {
                streaming_idat_chunk->bits = *(u16*)streaming_idat_chunk->at;
                Consume_(streaming_idat_chunk, 2);
                streaming_idat_chunk->bit_count = 0;
            }
        }
        else
        {
            u32 remaining_bits_count = (bit_count + streaming_idat_chunk->bit_count) - BITS_IN_TWO_BYTES;
            u32 start = streaming_idat_chunk->bit_count;
            result = GetBits(streaming_idat_chunk->bits, start, BITS_IN_TWO_BYTES - 1);
            
            streaming_idat_chunk->bits = *(u16*)streaming_idat_chunk->at;
            Consume_(streaming_idat_chunk, 2);
            streaming_idat_chunk->bit_count = 0;
            
            u32 remaining_bits = GetBits(streaming_idat_chunk->bits, 0, remaining_bits_count - 1);
            streaming_idat_chunk->bit_count += remaining_bits_count;
            
            u32 shift = BITS_IN_TWO_BYTES - start;
            
            remaining_bits <<= shift;
            
            result |= remaining_bits;
        }
    }
    
    return result;
}

internal u8
PaethPredictor(u8 left, u8 above, u8 upper_left)
{
    u8 result;
    
    s32 pa = abs(above - upper_left);
    s32 pb = abs(left - upper_left);
    s32 pc = abs((left - upper_left) + (above - upper_left));
    
    if((pa <= pb) && (pa <= pc))
    {
        result = left;
    }
    else if(pb <= pc)
    {
        result = above;
    }
    else
    {
        result = upper_left;
    }
    
    return result;
};

internal Bitmap
PNGFiltering(PNGData* png_data)
{
    Bitmap bitmap;
    bitmap.header.width = png_data->width - 1;
    bitmap.header.height = png_data->height;
    bitmap.header.bytes_per_pixel = png_data->bytes_per_pixel;
    
    bitmap.size = (bitmap.header.width * bitmap.header.height * bitmap.header.bytes_per_pixel) + sizeof(BitmapHeader);
    bitmap.buffer = malloc(bitmap.size);
    
    u8* bitmap_at = (u8*)bitmap.buffer;
    *(BitmapHeader*)bitmap_at = bitmap.header;
    bitmap_at += sizeof(BitmapHeader);
    u8* bitmap_start_at = bitmap_at;
    
    PNGFilteringMethod filtering_method = PNGFilteringMethod::Unfiltered;
    u8* row = 0;
    u8* prev_row = 0;
    u8* at = (u8*)png_data->buffer;
    u8 value = 0;
    s32 left_value = 0;
    s32 upper_left_value;
    
    for(u32 height_index = 0; height_index < png_data->height; height_index++)
    {
        u32 png_data_index = height_index * (((png_data->width - 1) * png_data->bytes_per_pixel) + 1);
        u32 bitmap_index = height_index * ((png_data->width - 1) *  png_data->bytes_per_pixel);
        
        filtering_method = (PNGFilteringMethod)at[png_data_index];
        
        printf("%d ", filtering_method);
        
        prev_row = row;
        row = at + png_data_index;
        row++;
        bitmap_at = bitmap_start_at + bitmap_index;
        
        for(u32 width_index = 0; width_index < ((png_data->width - 1) * png_data->bytes_per_pixel); width_index++)
        {
            value = row[width_index];
            
            switch(filtering_method)
            {
                case Unfiltered:
                {
                    value = value;
                } break;
                
                case Sub:
                {
                    if(width_index < png_data->bytes_per_pixel)
                    {
                        left_value = 0;
                    }
                    else
                    {
                        left_value = row[width_index - png_data->bytes_per_pixel];
                    }
                    
                    value += left_value;
                    row[width_index] = value;
                } break;
                
                case Up:
                {
                    value += prev_row[width_index];
                    row[width_index] = value;
                } break;
                
                case Average:
                {
                    if(width_index < png_data->bytes_per_pixel)
                    {
                        left_value = 0;
                    }
                    else
                    {
                        left_value = row[width_index - png_data->bytes_per_pixel];
                    }
                    
                    value += floor((left_value + prev_row[width_index]) / 2);
                    row[width_index] = value;
                } break;
                
                case Paeth:
                {
                    if(width_index < png_data->bytes_per_pixel)
                    {
                        left_value = 0;
                        upper_left_value = 0;
                    }
                    else
                    {
                        left_value = row[width_index - png_data->bytes_per_pixel];
                        upper_left_value = prev_row[width_index - png_data->bytes_per_pixel];
                    }
                    
                    value += PaethPredictor(left_value, prev_row[width_index], upper_left_value);
                    row[width_index] = value;
                    
                } break;
                
                default:
                {
                    printf("Filtering method: %d. Should be in range between 0 and 4", filtering_method);
                    Assert(false);
                } break;
            }
            
            bitmap_at[width_index] = value;
        }
    }
    
    return bitmap;
}

internal void
ConstructHuffmanCodes(HuffmanCodes* huffman_codes, u32* code_lengths, u32 code_lengths_size)
{
    for(u32 code_lengths_index = 0; code_lengths_index < code_lengths_size; code_lengths_index++)
    {
        u32 code_length = code_lengths[code_lengths_index];
        huffman_codes->code_lengths_repeat_count[code_length]++;
    }
    
    u32 lowest_values[BITS_IN_TWO_BYTES] = {};
    for(u32 length_index = 1; length_index < (BITS_IN_TWO_BYTES - 1); length_index++)
    {
        lowest_values[length_index + 1] = lowest_values[length_index] + huffman_codes->code_lengths_repeat_count[length_index];
    }
    
    for(u32 symbol_index = 0; symbol_index < code_lengths_size; symbol_index++)
    {
        if(code_lengths[symbol_index] != 0)
        {
            huffman_codes->symbols[lowest_values[code_lengths[symbol_index]]++] = symbol_index;
        }
    }
}

internal u16
DecodeHuffmanSymbol(HuffmanCodes* huffman_codes, StreamingIDATChunk* streaming_idat_chunk)
{
    s32 code = 0;
    s32 count = 0;
    s32 first = 0;
    u32 symbol = 0;
    u32 index = 0;
    for(u32 bit_index = 1; bit_index < BITS_IN_TWO_BYTES; bit_index++)
    {
        code |= ConsumeBits(streaming_idat_chunk, 1);
        count = huffman_codes->code_lengths_repeat_count[bit_index];
        if(code - count < first)
        {
            return huffman_codes->symbols[index + (code - first)];
        }
        
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    
    printf("Symbol was not found");
    Assert(false);
    return 0;
}

internal void 
DecodeLiteralAndDistanceCodes(HuffmanCodes* literal_codes, HuffmanCodes* distance_codes, StreamingIDATChunk* streaming_idat_chunk, PNGData* png_data)
{
    u16 lengths_base_size[29] = 
    {
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258,
    };
    
    u16 lengths_extra_bits[29] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0,
    };
    
    u16 distance_base_offset[30] = 
    {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577,
    };
    
    u16 distance_extra_bits[30] =
    {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13,
    };
    
    u16 symbol = 0;
    u32 index = 0;
    u32 length = 0;
    u32 distance = 0;
    
    do
    {
        symbol = DecodeHuffmanSymbol(literal_codes, streaming_idat_chunk);
        
        if(symbol < 256)
        {
            *png_data->at = (u8)symbol;
            png_data->at++;
            
            index++;
        }
        else if(symbol > 256)
        {
            symbol -= 257;
            length = lengths_base_size[symbol] + ConsumeBits(streaming_idat_chunk, lengths_extra_bits[symbol]);
            
            symbol = DecodeHuffmanSymbol(distance_codes, streaming_idat_chunk); 
            
            distance = distance_base_offset[symbol] + ConsumeBits(streaming_idat_chunk, distance_extra_bits[symbol]);
            
            u8* at_backward = png_data->at;
            at_backward -= distance;
            while(length--)
            {
                symbol = *at_backward++;
                *png_data->at = symbol;
                png_data->at++;
                index++;
                
            }
        }
        
    } while(symbol != 256);
}

internal void
DynamicHuffmanCodes(StreamingIDATChunk* streaming_idat_chunk, PNGData* png_data)
{
    u32 hlit = ConsumeBits(streaming_idat_chunk, 5);
    u32 hdist = ConsumeBits(streaming_idat_chunk, 5);
    u32 hclen = ConsumeBits(streaming_idat_chunk, 4);
    
    hlit += 257;
    hdist += 1;
    hclen += 4;
    
    printf("HLIT: %u\n", hlit);
    printf("HDIST: %u\n", hdist);
    printf("HCLEN: %u\n", hclen);
    
    u32 hclen_swizzle[] = 
    {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };
    
    u32 hclen_table_size = ArrayCount(hclen_swizzle);
    u32 hclen_table[ArrayCount(hclen_swizzle)] = {};
    
    for(u32 code_lengths_index = 0; code_lengths_index < hclen; code_lengths_index++)
    {
        hclen_table[hclen_swizzle[code_lengths_index]] = ConsumeBits(streaming_idat_chunk, 3);
    }
    
    u16 code_lengths_repeat_count[BITS_IN_TWO_BYTES] = {};
    u16 symbols[ArrayCount(hclen_swizzle)] = {};
    HuffmanCodes huffman_codes;
    huffman_codes.code_lengths_repeat_count = code_lengths_repeat_count;
    huffman_codes.symbols = symbols;
    
    ConstructHuffmanCodes(&huffman_codes, hclen_table, hclen_table_size);
    
    u32 lengths[MAX_CODES] = {};
    
    u32 repeat_symbol;
    u32 repeat_count;
    
    for(u32 index = 0; index < hlit + hdist;)
    {
        u16 symbol = DecodeHuffmanSymbol(&huffman_codes, streaming_idat_chunk);
        
        if(symbol <= 15)
        {
            lengths[index++] = symbol;
        }
        else
        {
            if(symbol == 16)
            {
                symbol = lengths[index - 1];
                repeat_count = 3 + ConsumeBits(streaming_idat_chunk, 2);
            }
            else if(symbol == 17)
            {
                symbol = 0;
                repeat_count = 3 + ConsumeBits(streaming_idat_chunk, 3);
            }
            else if(symbol == 18)
            {
                symbol = 0;
                repeat_count = 11 + ConsumeBits(streaming_idat_chunk, 7);
            }
            else
            {
                printf("Symbol: %u. Should be in range between 0 and 18", symbol);
                Assert(false);
            }
            while(repeat_count--)
            {
                lengths[index++] = symbol;
            }
        }
    }
    
    u16 literal_symbols[MAX_LITERAL_CODES] = {};
    u16 literal_repeat_count[BITS_IN_TWO_BYTES] = {};
    HuffmanCodes literal_codes;
    literal_codes.symbols = literal_symbols;
    literal_codes.code_lengths_repeat_count = literal_repeat_count;
    
    u16 distance_symbols[MAX_DISTANCE_CODES] = {};
    u16 distance_repeat_count[BITS_IN_TWO_BYTES] = {};
    HuffmanCodes distance_codes;
    distance_codes.symbols = distance_symbols;
    distance_codes.code_lengths_repeat_count = distance_repeat_count;
    
    ConstructHuffmanCodes(&literal_codes, lengths, hlit);
    ConstructHuffmanCodes(&distance_codes, lengths + hlit, hdist);
    
    DecodeLiteralAndDistanceCodes(&literal_codes, &distance_codes, streaming_idat_chunk, png_data);
}

internal Bitmap
ParseIDATChunks(StreamingIDATChunk* streaming_idat_chunk_head, PNGData* png_data)
{
    PNGChunkIDAT png_chunk_idat = Consume(PNGChunkIDAT, streaming_idat_chunk_head);
    
    u8 cm = GetBits(png_chunk_idat.zlib_compression_method, 0, 3);
    u8 cinfo = GetBits(png_chunk_idat.zlib_compression_method, 4, 7);
    u8 fcheck = GetBits(png_chunk_idat.additional_flags, 0, 4);
    u8 fdict = GetBits(png_chunk_idat.additional_flags, 5);
    u8 flevel = GetBits(png_chunk_idat.additional_flags, 6, 7);
    
    printf("    CM: %u\n", cm);
    printf("    CINFO: %u\n", cinfo);
    printf("    FCHECK: %u\n", fcheck);
    printf("    FDICT: %u\n", fdict);
    printf("    FLEVEL: %u\n", flevel);
    
    bool supported = false;
    
    if(cm == 8 &&
       cinfo <= 7 &&
       fdict == 0 &&
       flevel <= 3)
    {
        supported = true;
    }
    else
    {
        printf("Unsupported format");
    }
    
    Bitmap bitmap;
    
    if(supported)
    {
        png_data->buffer = malloc(png_data->height * (png_data->width + 1) * png_data->bytes_per_pixel);
        
        png_data->at = (u8*)png_data->buffer;
        
        streaming_idat_chunk_head->bits = *(u16*)streaming_idat_chunk_head->at;
        Consume_(streaming_idat_chunk_head, 2);
        
        u8 bfinal = 0;
        u8 btype = 0;
        
        while(bfinal == 0)
        {
            bfinal = ConsumeBits(streaming_idat_chunk_head, 1);
            btype = ConsumeBits(streaming_idat_chunk_head, 2);
            
            printf("    BFINAL: %u\n", bfinal);
            printf("    BTYPE: %u\n", btype);
            
            DynamicHuffmanCodes(streaming_idat_chunk_head, png_data);
        }
        
        bitmap = PNGFiltering(png_data);
        free(png_data->buffer);
    }
    
    return bitmap;
}

internal Bitmap
ParsePNG(FileContents* file_contents)
{
    StreamingBuffer streaming_buffer = {};
    streaming_buffer.at = (u8*)file_contents->buffer;
    streaming_buffer.size = file_contents->size;
    
    StreamingIDATChunk* streaming_idat_chunk_head = 0;
    StreamingIDATChunk* streaming_idat_chunk_last = 0;
    
    Bitmap bitmap;
    
    PNGData png_data = {};
    png_data.bytes_per_pixel = 4;
    
    PNGHeader png_header = Consume(PNGHeader, &streaming_buffer);
    if(!memcmp(png_header.signature, png_signature, ArrayCount(png_signature)))
    {
        printf("\n");
        while(streaming_buffer.size > 0)
        {
            PNGChunkHeader png_chunk_header = Consume(PNGChunkHeader, &streaming_buffer);
            
            printf("%c%c%c%c\n", png_chunk_header.type_in_bytes[0],
                   png_chunk_header.type_in_bytes[1], 
                   png_chunk_header.type_in_bytes[2],
                   png_chunk_header.type_in_bytes[3]);
            
            ChangeEndianness(&png_chunk_header.length);
            ChangeEndianness(&png_chunk_header.type);
            
            if(png_chunk_header.type == 'IHDR')
            {
                PNGChunkIHDR png_chunk_ihdr = Consume(PNGChunkIHDR, &streaming_buffer);
                ChangeEndianness(&png_chunk_ihdr.width);
                ChangeEndianness(&png_chunk_ihdr.height);
                
                png_data.width = png_chunk_ihdr.width + 1;
                png_data.height = png_chunk_ihdr.height;
                
                printf("    width: %u\n", png_chunk_ihdr.width);
                printf("    height: %u\n", png_chunk_ihdr.height);
                printf("    depth: %u\n", png_chunk_ihdr.bit_depth);
                printf("    color_type: %u\n", png_chunk_ihdr.color_type);
                printf("    compression: %u\n", png_chunk_ihdr.compression_method);
                printf("    filter: %u\n", png_chunk_ihdr.filter_method);
                printf("    interlace: %u\n", png_chunk_ihdr.interlace_method);
            }
            else
            {
                if(png_chunk_header.type == 'IDAT')
                {
                    StreamingIDATChunk* streaming_idat_chunk = (StreamingIDATChunk*)calloc(1, sizeof(StreamingIDATChunk));
                    streaming_idat_chunk->at = streaming_buffer.at;
                    streaming_idat_chunk->size = png_chunk_header.length;
                    
                    if(streaming_idat_chunk_head)
                    {
                        streaming_idat_chunk_last->next = streaming_idat_chunk;
                        streaming_idat_chunk_last = streaming_idat_chunk;
                    }
                    else
                    {
                        streaming_idat_chunk_head = streaming_idat_chunk;
                        streaming_idat_chunk_last = streaming_idat_chunk_head;
                    }
                }
                Consume_(&streaming_buffer, png_chunk_header.length);
            }
            
            PNGChunkFooter png_chunk_footer = Consume(PNGChunkFooter, &streaming_buffer);
        }
        
        bitmap = ParseIDATChunks(streaming_idat_chunk_head, &png_data);
    }
    else
    {
        printf("Incorrect PNG signature");
    }
    
    return bitmap;
}

internal FileContents 
ReadEntireFile(char* file_path)
{
    FileContents result = {};
    
    FILE* handle = fopen(file_path, "rb");
    if(handle)
    {
        fseek(handle, 0, SEEK_END);
        s64 bytes_to_read = ftell(handle);
        fseek(handle, 0, SEEK_SET);
        
        void* buffer = malloc(bytes_to_read);
        s64 bytes_read = fread(buffer, 1, bytes_to_read, handle);
        if(bytes_to_read == bytes_read)
        {
            result.buffer = buffer;
            result.size = bytes_to_read;
        }
        else
        {
            free(buffer);
        }
    }
    
    return result;
}

int main(int argc, char* argv[])
{
    char* file_path = argv[1];
    bool correct_arguments = true;
    bool show_details = true;
    
    if(argc > 1)
    {
        if(argc == 2)
        {
            file_path = argv[1];
        }
        else if(argc == 3)
        {
            file_path = argv[2];
            
            char* options = argv[1];
            size_t options_length = strlen(options);
            
            if(options_length > 2)
            {
                if((options[0] == '-') &&
                   (options[1] == 'q'))
                {
                    correct_arguments = true;
                    show_details = false;
                }
                else
                {
                    correct_arguments = false;
                }
            }
            else
            {
                correct_arguments = false;
            }
        }
        
    }
    else
    {
        correct_arguments = false;
    }
    
    if(correct_arguments)
    {
        if(show_details)
        {
            printf("Parsing a file: \"%s\"\n", file_path);
        }
        
        FileContents file_contents = ReadEntireFile(file_path);
        Bitmap bitmap = ParsePNG(&file_contents);
        
        size_t file_path_size = strlen(file_path);
        size_t index = file_path_size;
        while(index)
        {
            if(file_path[index] == '.')
            {
                break;
            }
            else
            {
                index--;
            }
        }
        
        char* extension = "rgba";
        size_t extension_size = strlen(extension);
        
        char* file_path_output = (char*)malloc(index + extension_size + 1);
        file_path_output = strncpy(file_path_output, file_path, index + 1);
        strcat(file_path_output, extension);
        
        FILE* file_output = fopen(file_path_output, "wb");
        fwrite(bitmap.buffer, 1, bitmap.size, file_output);
        fclose(file_output);
    }
    else
    {
        char* help = 
            "\nUsage:  parser [-q] file.{png}\n\n"
            "Options:\n"
            "   -q test quietly\n";
        
        printf("%s", help);
    }
    
    return 0;
}

