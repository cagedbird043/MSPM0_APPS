import sys
import json
import re

def main():
    if len(sys.argv) != 4:
        print(f"Usage: python {sys.argv[0]} <board_json_path> <input_syscfg_path> <output_syscfg_path>")
        sys.exit(1)

    board_json_path, input_syscfg_path, output_syscfg_path = sys.argv[1], sys.argv[2], sys.argv[3]

    try:
        with open(board_json_path, 'r', encoding='utf-8') as f:
            json_with_comments = f.read()
        json_string = re.sub(r'/\*.*?\*/', '', json_with_comments, flags=re.DOTALL)
        board_data = json.loads(json_string)

        with open(input_syscfg_path, 'r', encoding='utf-8') as f:
            syscfg_content = f.read()

        aliases_to_find = re.findall(r'"(boosterpack\.\d+|j\d+_expansion\.\d+)"', syscfg_content)
        
        print(f"--- Found Aliases to Replace in '{input_syscfg_path}' ---")
        print(list(set(aliases_to_find)))
        print("----------------------------------------------------")

        # 【关键修正】将 headers 和 connectors 两个列表合并成一个来查找！
        all_definitions = board_data.get("headers", []) + board_data.get("connectors", [])

        for alias in set(aliases_to_find):
            connector_name, pin_number_str = alias.split('.')
            pin_number_int = int(pin_number_str)
            
            physical_pin = None
            
            # 遍历合并后的完整定义列表
            for connector in all_definitions:
                if connector.get("name") == connector_name:
                    for pin_info in connector.get("pins", []):
                        if pin_info.get("number") == pin_number_int:
                            physical_pin = pin_info.get("name")
                            break
                    if physical_pin:
                        break
            
            if physical_pin:
                print(f"SUCCESS: Mapping for '{alias}' -> '{physical_pin}'. Replacing...")
                syscfg_content = syscfg_content.replace(f'"{alias}"', f'"{physical_pin}"')
            else:
                print(f"!!! FAIL: Could not find mapping for '{alias}' in the JSON data structure.")

        with open(output_syscfg_path, 'w', encoding='utf-8') as f:
            f.write(syscfg_content)
            
        print(f"\nSuccessfully preprocessed syscfg file to: {output_syscfg_path}")

    except Exception as e:
        print(f"\nERROR during preprocessing: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()